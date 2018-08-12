/*****************************************************************************
* \file      apx_fileManagerReceiver.c
* \author    Conny Gustafsson
* \date      2018-08-02
* \brief     APX Filemanager receiver component
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
#include "apx_fileManagerRemote.h"
#include "apx_logging.h"
#include "apx_eventFile.h"
#include "rmf.h"

//temporary includes
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
#define DYN_STATIC
#else
#define DYN_STATIC static
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
DYN_STATIC void apx_fileManagerRemote_processCmdMsg(apx_fileManagerRemote_t *self, const uint8_t *msgBuf, int32_t msgLen);
DYN_STATIC void apx_fileManagerRemote_processDataMsg(apx_fileManagerRemote_t *self, uint32_t address, const uint8_t *msgBuf, int32_t msgLen, bool more_bit);
static void apx_fileManagerRemote_processFileInfo(apx_fileManagerRemote_t *self, const rmf_fileInfo_t *cmdFileInfo);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_fileManagerRemote_create(apx_fileManagerRemote_t *self, apx_fileManagerShared_t *shared)
{
   if (self != 0)
   {
      MUTEX_INIT(self->mutex);
      apx_fileMap_create(&self->remoteFileMap);
      self->shared = shared;
   }
}

void apx_fileManagerRemote_destroy(apx_fileManagerRemote_t *self)
{
   if (self != 0)
   {
      apx_fileMap_destroy(&self->remoteFileMap);
      MUTEX_DESTROY(self->mutex);
   }
}

int32_t apx_fileManagerRemote_parseMessage(apx_fileManagerRemote_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   rmf_msg_t msg;
   int32_t result = rmf_unpackMsg(msgBuf, msgLen, &msg);
   if (result > 0)
   {
#if APX_FILEMANAGER_DEBUG_ENABLE
      APX_LOG_DEBUG("[APX_FILE_MANAGER] address: %08X", msg.address);
      APX_LOG_DEBUG("[APX_FILE_MANAGER] length: %d", msg.dataLen);
      APX_LOG_DEBUG("[APX_FILE_MANAGER] more_bit: %d", (int) msg.more_bit);
#endif
      if (msg.address == RMF_CMD_START_ADDR)
      {
         apx_fileManagerRemote_processCmdMsg(self, msg.data, msg.dataLen);
      }
      else if (msg.address < RMF_CMD_START_ADDR)
      {
         apx_fileManagerRemote_processDataMsg(self, msg.address, msg.data, msg.dataLen, msg.more_bit);
      }
      else
      {
         //discard
      }
   }
   else if (result < 0)
   {
      APX_LOG_ERROR("[APX_FILE_MANAGER] rmf_unpackMsg failed with %d", (int)result);
   }
   else
   {
      //MISRA
   }
   return result;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
DYN_STATIC void apx_fileManagerRemote_processCmdMsg(apx_fileManagerRemote_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   if (self != 0)
   {
      uint32_t cmdType;
      int32_t result;
      result = rmf_deserialize_cmdType(msgBuf, msgLen, &cmdType);
      if (result > 0)
      {
         switch(cmdType)
         {
            case RMF_CMD_FILE_INFO:
               {
                  rmf_fileInfo_t cmdFileInfo;
                  result = rmf_deserialize_cmdFileInfo(msgBuf, msgLen, &cmdFileInfo);
                  if (result > 0)
                  {
                     apx_fileManagerRemote_processFileInfo(self, &cmdFileInfo);
                  }
                  else if (result < 0)
                  {
                     APX_LOG_ERROR("[APX_FILE_MANAGER] rmf_deserialize_cmdFileInfo failed with %d", (int) result);
                  }
                  else
                  {
                     APX_LOG_ERROR("[APX_FILE_MANAGER] rmf_deserialize_cmdFileInfo returned 0");
                  }
               }
               break;
            case RMF_CMD_FILE_OPEN:
               {
                  rmf_cmdOpenFile_t cmdOpenFile;
                  result = rmf_deserialize_cmdOpenFile(msgBuf, msgLen, &cmdOpenFile);
                  if (result > 0)
                  {
                     assert(self->shared->fileOpenRequestedByRemote!=0);
                     self->shared->fileOpenRequestedByRemote(self->shared->arg, &cmdOpenFile);
                  }
                  else if (result < 0)
                  {
                     APX_LOG_ERROR("[APX_FILE_MANAGER] rmf_deserialize_cmdOpenFile failed with %d", (int) result);
                  }
                  else
                  {
                     APX_LOG_ERROR("[APX_FILE_MANAGER] rmf_deserialize_cmdOpenFile returned 0");
                  }
               }
               break;
            case RMF_CMD_HEARTBEAT_RQST:
               ///TODO: implement
               break;
            case RMF_CMD_HEARTBEAT_RSP:
               ///TODO: implement
               break;
            case RMF_CMD_PING_RQST:
               ///TODO: implement
               break;
            case RMF_CMD_PING_RSP:
               ///TODO: implement
               break;

            default:
               APX_LOG_ERROR("[APX_FILE_MANAGER] not implemented cmdType: %d\n", cmdType);
         }
      }
   }
}

DYN_STATIC void apx_fileManagerRemote_processDataMsg(apx_fileManagerRemote_t *self, uint32_t address, const uint8_t *msgBuf, int32_t msgLen, bool more_bit)
{

}

static void apx_fileManagerRemote_processFileInfo(apx_fileManagerRemote_t *self, const rmf_fileInfo_t *cmdFileInfo)
{
   if ( (self != 0) && (cmdFileInfo != 0) )
   {
      apx_file_t *remoteFile = apx_file_new(APX_UNKNOWN_FILE, cmdFileInfo);
      if (remoteFile != 0)
      {
         MUTEX_LOCK(self->mutex);
         apx_fileMap_insertFile(&self->remoteFileMap, remoteFile);
         MUTEX_UNLOCK(self->mutex);
         if(strcmp(remoteFile->fileInfo.name, APX_EVENT_LOG_FILE_NAME)==0)
         {
            printf("event file seen\n");
         }
         else
         {
            if (self->shared->fileCreated != 0)
            {
               self->shared->fileCreated(self->shared->arg, remoteFile);
            }
         }
      }
      else
      {
         APX_LOG_ERROR("[APX_FILE_MANAGER] apx_file_newRemoteFile returned NULL");
      }
   }
}



