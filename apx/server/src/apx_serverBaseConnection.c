/*****************************************************************************
* \file      apx_srvBaseConnection.c
* \author    Conny Gustafsson
* \date      2018-09-26
* \brief     Description
*
* Copyright (c) 2018 Conny Gustafsson
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "apx_serverBaseConnection.h"
#include "bstr.h"
#include "headerutil.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_logging.h"
#include "apx_file2.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define MAX_HEADER_LEN 128
#define APX_EXTENSION ".apx"

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_serverBaseConnection_parseGreeting(apx_serverBaseConnection_t *self, const uint8_t *msgBuf, int32_t msgLen);
static uint8_t apx_serverBaseConnection_parseMessage(apx_serverBaseConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
static void apx_serverBaseConnection_onFileCreate(void *arg, apx_fileManager_t *fileManager, struct apx_file2_tag *file);
static void apx_serverBaseConnection_processNewApxFile(apx_serverBaseConnection_t *self, struct apx_file2_tag *file);
static apx_error_t apx_serverBaseConnection_writeDefinitionFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

apx_error_t apx_serverBaseConnection_create(apx_serverBaseConnection_t *self, uint32_t connectionId, struct apx_server_tag *server, void (*destructor)(void *arg))
{
   if (self != 0)
   {
      int8_t result;
      self->connectionId = connectionId;
      self->server = server;
      self->vtable.destructor = destructor;
      self->numHeaderLen = (int8_t) sizeof(uint32_t);
      apx_nodeDataManager_create(&self->nodeDataManager);
      result = apx_fileManager_create(&self->fileManager, APX_FILEMANAGER_SERVER_MODE, connectionId);
      if (result != 0)
      {
         apx_nodeDataManager_destroy(&self->nodeDataManager);
      }
      return result;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_serverBaseConnection_destroy(apx_serverBaseConnection_t *self)
{
   if (self != 0)
   {
      apx_fileManager_destroy(&self->fileManager);
      apx_nodeDataManager_destroy(&self->nodeDataManager);
   }
}

void apx_serverBaseConnection_delete(apx_serverBaseConnection_t *self)
{
   if(self != 0)
   {
      if (self->vtable.destructor != 0)
      {
         self->vtable.destructor((void*) self);
      }
      free(self);
   }
}

void apx_serverBaseConnection_vdelete(void *arg)
{
   apx_serverBaseConnection_delete((apx_serverBaseConnection_t*) arg);
}

apx_fileManager_t *apx_serverBaseConnection_getFileManager(apx_serverBaseConnection_t *self)
{
   if (self != 0)
   {
      return &self->fileManager;
   }
   return (apx_fileManager_t*) 0;
}

int8_t apx_serverBaseConnection_dataReceived(apx_serverBaseConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   if ( (self != 0) && (dataBuf != 0) && (parseLen != 0) )
   {
      uint32_t totalParseLen = 0;
      uint32_t remain = dataLen;
      const uint8_t *pNext = dataBuf;
      while(totalParseLen<dataLen)
      {
         uint32_t internalParseLen = 0;
         uint8_t result;
         result = apx_serverBaseConnection_parseMessage(self, pNext, remain, &internalParseLen);
         //check parse result
         if (result == 0)
         {
            //printf("\tinternalParseLen(%s): %d\n",parseFunc, internalParseLen);
            assert(internalParseLen<=dataLen);
            pNext+=internalParseLen;
            totalParseLen+=internalParseLen;
            remain-=internalParseLen;
            if(internalParseLen == 0)
            {
               break;
            }
         }
         else
         {
            return result;
         }
      }
      //no more complete messages can be parsed. There may be a partial message left in buffer, but we ignore it until more data has been recevied.
      //printf("\ttotalParseLen=%d\n", totalParseLen);
      *parseLen = totalParseLen;
      return 0;
   }
   return -1;
}

void apx_serverBaseConnection_start(apx_serverBaseConnection_t *self)
{
   if ( self != 0)
   {
      apx_fileManagerEventListener_t listener;
      memset(&listener, 0, sizeof(listener));
      listener.fileCreate = apx_serverBaseConnection_onFileCreate;
      listener.arg = (void*) self;
      apx_fileManager_start(&self->fileManager);
      apx_fileManager_registerEventListener(&self->fileManager, &listener);
   }
}


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_serverBaseConnection_parseGreeting(apx_serverBaseConnection_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   const uint8_t *pNext = msgBuf;
   const uint8_t *pEnd = msgBuf + msgLen;
   while(pNext < pEnd)
   {
      const uint8_t *pResult;
      pResult = bstr_line(pNext, pEnd);
      if ( (pResult > pNext) || ((pResult==pNext) && *pNext==(uint8_t) '\n') )
      {
         //found a line ending with '\n'
         const uint8_t *pMark = pNext;
         int32_t lengthOfLine = (int32_t) (pResult-pNext);
         //move pNext to beginning of next line (one byte after the '\n')
         pNext = pResult+1;
         if (lengthOfLine == 0)
         {
            //this ends the header
            self->isGreetingParsed = true;
#if 0
            if (self->debugMode > APX_DEBUG_NONE)
            {
               APX_LOG_INFO("[APX_SRV_CONNECTION] (%p) Greeting parsed", (void*) self);
            }
            else
            {
               APX_LOG_INFO("%s", "[APX_SRV_CONNECTION] Greeting parsed");
            }
#endif
            apx_fileManager_onHeaderReceived(&self->fileManager);
            break;
         }
         else
         {
            //TODO: parse greeting line
            if (lengthOfLine<MAX_HEADER_LEN)
            {
               char tmp[MAX_HEADER_LEN+1];
               memcpy(tmp,pMark,lengthOfLine);
               tmp[lengthOfLine]=0;
               //printf("\tgreeting-line: '%s'\n",tmp);
            }
         }
      }
      else
      {
         break;
      }
   }
}

/**
 * a message consists of a message length (1 or 4 bytes) packed as binary integer (big endian). Then follows the message data followed by a new message length header etc.
 */
static uint8_t apx_serverBaseConnection_parseMessage(apx_serverBaseConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   uint32_t totalParsed=0;
   uint32_t msgLen;
   const uint8_t *pBegin = dataBuf;
   const uint8_t *pResult;
   const uint8_t *pEnd = dataBuf+dataLen;
   const uint8_t *pNext = pBegin;
   pResult = headerutil_numDecode32(pNext, pEnd, &msgLen);
   if (pResult>pNext)
   {
      uint32_t headerLen = (uint32_t) (pResult-pNext);
      pNext+=headerLen;
      if (pNext+msgLen<=pEnd)
      {
         totalParsed+=headerLen+msgLen;
#if 0
         if (self->debugMode >= APX_DEBUG_4_HIGH)
         {

            uint32_t i;
            char msg[MAX_DEBUG_MSG_SIZE];
            char *pMsg = &msg[0];
            char *pMsgEnd = pMsg + MAX_DEBUG_MSG_SIZE;
            pMsg += sprintf(msg, "(%p) Received %d+%d bytes:", (void*)self, (int)headerLen, (int)msgLen);
            for (i = 0; i < MAX_DEBUG_BYTES; i++)
            {
               if ( ( i >= (msgLen + headerLen) ) || ( (pMsg + HEX_DATA_LEN) > pMsgEnd))
               {
                  break;
               }
               pMsg += sprintf(pMsg, " %02X", (int)pBegin[i]);
            }
            APX_LOG_DEBUG("[APX_SRV_CONNECTION] %s", msg);
         }
#endif
         if (self->isGreetingParsed == false)
         {
            apx_serverBaseConnection_parseGreeting(self, pNext, msgLen);
         }
         else
         {
            apx_fileManager_processMessage(&self->fileManager, pNext, msgLen);
         }
      }
      else
      {
         //we have to wait until entire message is in the buffer
      }
   }
   else
   {
      //there is not enough bytes in buffer to parse header
   }
   *parseLen=totalParsed;
   return 0;
}

static void apx_serverBaseConnection_onFileCreate(void *arg, apx_fileManager_t *fileManager, struct apx_file2_tag *file)
{
   apx_serverBaseConnection_t *self = (apx_serverBaseConnection_t*) arg;
   if (self != 0)
   {
      if ( strcmp(apx_file2_extension(file), APX_EXTENSION) == 0)
      {
         apx_serverBaseConnection_processNewApxFile(self,  file);
      }
   }
}

#include <stdio.h>
static void apx_serverBaseConnection_processNewApxFile(apx_serverBaseConnection_t *self, struct apx_file2_tag *file)
{
   if (file->fileInfo.fileType == RMF_FILE_TYPE_FIXED)
   {
      if (apx_nodeDataManager_find(&self->nodeDataManager, file->fileInfo.name) != 0)
      {
         APX_LOG_WARNING("APX node already exits: %s", file->fileInfo.name);
         apx_fileManager_sendFileAlreadyExistsError(&self->fileManager, file);
      }
      else
      {
         apx_nodeData_t *nodeData = apx_nodeData_new(file->fileInfo.length);
         if (nodeData != 0)
         {
            apx_file_handler_t fileHandler;
            memset(&fileHandler, 0, sizeof(fileHandler));
            fileHandler.arg = self;
            fileHandler.write = apx_serverBaseConnection_writeDefinitionFile;
            apx_file2_setHandler(file, &fileHandler);
            apx_nodeData_setDefinitionFile(nodeData, file);
            assert(apx_nodeDataManager_attach(&self->nodeDataManager, nodeData) == APX_NO_ERROR);
            apx_fileManager_openRemoteFile(&self->fileManager, file->fileInfo.address, (void*) self);
         }
      }
   }
}

static apx_error_t apx_serverBaseConnection_writeDefinitionFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more)
{
   //printf("address=%d\n", file->fileInfo.address);
   return APX_NO_ERROR;
}
