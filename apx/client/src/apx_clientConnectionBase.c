/*****************************************************************************
* \file      apx_clientConnectionBase.c
* \author    Conny Gustafsson
* \date      2018-12-31
* \brief     Base class for client connections
*
* Copyright (c) 2018-2019 Conny Gustafsson
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
#include <stdio.h> //debug only
#include "apx_clientConnectionBase.h"
#include "bstr.h"
#include "numheader.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_logging.h"
#include "apx_file2.h"
#include "rmf.h"
#include "apx_portDataMap.h"
#include "apx_server.h"
#include "apx_routingTable.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define MAX_HEADER_LEN 128
//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_clientConnectionBase_onFileCreate(void *arg, apx_fileManager_t *fileManager, struct apx_file2_tag *file);
static uint8_t apx_clientConnectionBase_parseMessage(apx_clientConnectionBase_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
static void apx_clientConnectionBase_processNewInDataFile(apx_clientConnectionBase_t *self, struct apx_file2_tag *file);
static void apx_clientConnectionBase_sendGreeting(apx_clientConnectionBase_t *self);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_clientConnectionBase_create(apx_clientConnectionBase_t *self, struct apx_client_tag *client, apx_connectionBaseVTable_t *vtable)
{
   if (self != 0)
   {
      apx_error_t result = apx_connectionBase_create(&self->base, APX_CLIENT_MODE, vtable);
      self->client = client;
      apx_connectionBase_setEventHandler(&self->base, apx_clientConnectionBase_defaultEventHandler, (void*) self);
      return result;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_clientConnectionBase_destroy(apx_clientConnectionBase_t *self)
{
   if (self != 0)
   {
      apx_connectionBase_destroy(&self->base);
   }
}

apx_fileManager_t *apx_clientConnectionBase_getFileManager(apx_clientConnectionBase_t *self)
{
   if (self != 0)
   {
      return &self->base.fileManager;
   }
   return (apx_fileManager_t*) 0;
}

void apx_clientConnectionBase_onConnected(apx_clientConnectionBase_t *self)
{
   apx_clientConnectionBase_sendGreeting(self);
   apx_connectionBase_start(&self->base);
}

void apx_clientConnectionBase_onDisconnected(apx_clientConnectionBase_t *self)
{

}

int8_t apx_clientConnectionBase_onDataReceived(apx_clientConnectionBase_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
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
         result = apx_clientConnectionBase_parseMessage(self, pNext, remain, &internalParseLen);
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

void apx_clientConnectionBase_start(apx_clientConnectionBase_t *self)
{
   if ( self != 0)
   {
      apx_fileManagerEventListener_t listener;
      memset(&listener, 0, sizeof(listener));
      listener.fileCreate = apx_clientConnectionBase_onFileCreate;
      listener.arg = (void*) self;
      apx_fileManager_start(&self->base.fileManager);
      apx_fileManager_registerEventListener(&self->base.fileManager, &listener);
   }
}

void apx_clientConnectionBase_defaultEventHandler(void *arg, apx_event_t *event)
{
   apx_clientConnectionBase_t *self = (apx_clientConnectionBase_t*) arg;
   if (self != 0)
   {
      apx_connectionBase_defaultEventHandler(&self->base, event);
   }
}

void apx_clientConnectionBase_setConnectionId(apx_clientConnectionBase_t *self, uint32_t connectionId)
{
   if (self != 0)
   {
      return apx_connectionBase_setConnectionId(&self->base, connectionId);
   }
}

uint32_t apx_clientConnectionBase_getConnectionId(apx_clientConnectionBase_t *self)
{
   if (self != 0)
   {
      return apx_connectionBase_getConnectionId(&self->base);
   }
   return 0;
}

void apx_clientConnectionBase_close(apx_clientConnectionBase_t *self)
{
   if (self != 0)
   {
      apx_connectionBase_close(&self->base);
   }
}

#ifdef UNIT_TEST
void apx_clientConnectionBase_run(apx_clientConnectionBase_t *self)
{
   if (self != 0)
   {
      apx_connectionBase_runAll(&self->base);
      apx_fileManager_run(&self->base.fileManager);
   }
}
#endif
//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_clientConnectionBase_onFileCreate(void *arg, apx_fileManager_t *fileManager, struct apx_file2_tag *file)
{
   apx_clientConnectionBase_t *self = (apx_clientConnectionBase_t*) arg;
   printf("file created: %s\n", file->fileInfo.name);
   if (self != 0)
   {
      if ( strcmp(apx_file2_extension(file), APX_INDATA_FILE_EXT) == 0)
      {
         apx_clientConnectionBase_processNewInDataFile(self,  file);
      }
   }
}

static uint8_t apx_clientConnectionBase_parseMessage(apx_clientConnectionBase_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   return 0;
}

static void apx_clientConnectionBase_processNewInDataFile(apx_clientConnectionBase_t *self, struct apx_file2_tag *file)
{

}

static void apx_clientConnectionBase_sendGreeting(apx_clientConnectionBase_t *self)
{
   uint8_t *sendBuffer;
   uint32_t greetingLen;
   apx_transmitHandler_t transmitHandler;
   int numheaderFormat = 32;
   char greeting[RMF_GREETING_MAX_LEN];
   char *p = &greeting[0];
   strcpy(greeting, RMF_GREETING_START);
   p += strlen(greeting);
   p += sprintf(p, "%s%d\n\n", RMF_NUMHEADER_FORMAT_HDR, numheaderFormat);
   greetingLen = (uint32_t) (p-greeting);
   apx_connectionBase_getTransmitHandler(&self->base, &transmitHandler);
   if ( (transmitHandler.getSendBuffer != 0) && (transmitHandler.send != 0) )
   {
      sendBuffer = transmitHandler.getSendBuffer((void*) self, greetingLen);
      if (sendBuffer != 0)
      {
         memcpy(sendBuffer, greeting, greetingLen);
         transmitHandler.send((void*) self, 0, greetingLen);
      }
      else
      {
         fprintf(stderr, "Failed to acquire sendBuffer while trying to send greeting\n");
      }
   }
}
