/*****************************************************************************
* \file      apx_fileManager.c
* \author    Conny Gustafsson
* \date      2018-08-04
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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_msg.h"
#include "apx_logging.h"


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_fileManager_triggerHeaderReceivedEvent(apx_fileManager_t *self);
static int8_t apx_fileManager_startWorkerThread(apx_fileManager_t *self);
static void apx_fileManager_stopWorkerThread(apx_fileManager_t *self);
static void apx_fileManager_triggerSendAcknowledge(apx_fileManager_t *self);

static void apx_fileManager_fileCreatedCbk(void *arg, const struct apx_file_tag *pFile);
static void apx_fileManager_sendFileInfoCbk(void *arg, const struct apx_file_tag *pFile);


static THREAD_PROTO(workerThread,arg);
static bool workerThread_processMessage(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendFileInfo(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendAcknowledge(apx_fileManager_t *self);
//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_fileManager_create(apx_fileManager_t *self, uint8_t mode, uint32_t connectionId)
{
   if (self != 0)
   {
      int8_t result;
      size_t numItems = APX_MAX_NUM_MESSAGES;
      size_t elemSize = RMF_MSG_SIZE;
      self->ringbufferLen = numItems;
      self->ringbufferData = (uint8_t*) malloc(numItems*elemSize);
      if (self->ringbufferData == 0)
      {
         return -1;
      }
      adt_rbfs_create(&self->messages, self->ringbufferData,(uint16_t) numItems,(uint8_t) elemSize);
      result = apx_fileManagerShared_create(&self->shared, connectionId);
      if (result == 0)
      {
         apx_fileManagerRemote_create(&self->remote, &self->shared);
         apx_fileManagerLocal_create(&self->local, &self->shared);
         adt_list_create(&self->eventListeners, apx_eventListener_vdelete);
         self->mode = mode;
         self->shared.arg = self;
         self->shared.fileCreated = apx_fileManager_fileCreatedCbk;
         self->shared.sendFileInfo = apx_fileManager_sendFileInfoCbk;
         MUTEX_INIT(self->mutex);
         SPINLOCK_INIT(self->lock);
         SEMAPHORE_CREATE(self->semaphore);
   #ifdef _WIN32
            self->workerThread = INVALID_HANDLE_VALUE;
   #else
            self->workerThread = 0;
   #endif
         self->workerThreadValid=false;
         self->headerSize = (uint8_t) sizeof(uint32_t);
         apx_fileManager_setTransmitHandler(self, 0);
      }
      else
      {
         free(self->ringbufferData);
      }
      return result;
   }
   return -1;
}

void apx_fileManager_destroy(apx_fileManager_t *self)
{
   if (self != 0)
   {
      if (self->workerThreadValid == true)
      {
         apx_fileManager_stop(self);
      }
      adt_list_destroy(&self->eventListeners);
      apx_fileManagerLocal_destroy(&self->local);
      apx_fileManagerRemote_destroy(&self->remote);
      apx_fileManagerShared_destroy(&self->shared);
      MUTEX_DESTROY(self->mutex);
      SPINLOCK_DESTROY(self->lock);
      if (self->ringbufferData != 0)
      {
         free(self->ringbufferData);
      }
   }
}

void* apx_fileManager_registerEventListener(apx_fileManager_t *self, struct apx_eventListener_tag* listener)
{
   if ( (self != 0) && (listener != 0))
   {
      void *handle = (void*) apx_eventListener_clone(listener);
      if (handle != 0)
      {
         adt_list_insert(&self->eventListeners, handle);
      }
      return handle;
   }
   return (void*) 0;
}

void apx_fileManager_unregisterEventListener(apx_fileManager_t *self, void *handle)
{
   if ( (self != 0) && (handle != 0))
   {
      bool isFound = adt_list_remove(&self->eventListeners, handle);
      if (isFound == true)
      {
         apx_eventListener_vdelete(handle);
      }
   }
}

int32_t apx_fileManager_getNumEventListeners(apx_fileManager_t *self)
{
   if (self != 0)
   {
      return adt_list_length(&self->eventListeners);
   }
   errno = EINVAL;
   return -1;
}

void apx_fileManager_attachLocalFile(apx_fileManager_t *self, struct apx_file_tag *localFile)
{
   if (self != 0)
   {
      apx_fileManagerLocal_attachFile(&self->local, localFile);
   }
}

int32_t apx_fileManager_getNumLocalFiles(apx_fileManager_t *self)
{
   if (self != 0)
   {
      return apx_fileManagerLocal_getNumFiles(&self->local);
   }
   errno = EINVAL;
   return -1;
}

bool apx_fileManager_isServerMode(apx_fileManager_t *self)
{
   if ( (self != 0) && (self->mode == APX_FILEMANAGER_SERVER_MODE))
   {
      return true;
   }
   return false;
}

int32_t apx_fileManager_parseMessage(apx_fileManager_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   if (self != 0)
   {
      return apx_fileManagerRemote_parseMessage(&self->remote, msgBuf, msgLen);
   }
   return -1;
}

void apx_fileManager_triggerNewConnectionEvent(apx_fileManager_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->newConnection != 0)
         {
            listener->newConnection(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->mutex);
   }
}

void apx_fileManager_start(apx_fileManager_t *self)
{
   if (self != 0)
   {
      apx_fileManager_startWorkerThread(self);
   }
}

void apx_fileManager_stop(apx_fileManager_t *self)
{
   if (self != 0)
   {
      apx_fileManager_stopWorkerThread(self);
   }
}
void apx_fileManager_onHeaderReceived(apx_fileManager_t *self)
{
   if (self != 0)
   {
      assert(self->mode == APX_FILEMANAGER_SERVER_MODE);
      apx_fileManager_triggerHeaderReceivedEvent(self);
      apx_fileManager_triggerSendAcknowledge(self);
      apx_fileManagerLocal_sendFileInfo(&self->local);
   }
}

/**
 * called by connection after it has successfully parsed the RMF header
 */
void apx_fileManager_onHeaderAccepted(apx_fileManager_t *self)
{
   if (self != 0)
   {
      assert(self->mode == APX_FILEMANAGER_CLIENT_MODE);
      apx_fileManagerLocal_sendFileInfo(&self->local);
   }
}

uint32_t fileManager_getID(apx_fileManager_t *self)
{
   if (self != 0)
   {
      return self->shared.fmid;
   }
   return 0;
}

void apx_fileManager_setTransmitHandler(apx_fileManager_t *self, apx_transmitHandler_t *handler)
{
   if (self != 0)
   {
      SPINLOCK_ENTER(self->lock);
      if (handler == 0)
      {
         memset(&self->transmitHandler, 0, sizeof(apx_transmitHandler_t));
      }
      else
      {
         memcpy(&self->transmitHandler, handler, sizeof(apx_transmitHandler_t));
      }
      SPINLOCK_LEAVE(self->lock);
   }
}


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void apx_fileManager_triggerHeaderReceivedEvent(apx_fileManager_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->headerReceived != 0)
         {
            listener->headerReceived(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->mutex);
   }
}

static int8_t apx_fileManager_startWorkerThread(apx_fileManager_t *self)
{
   if( self->workerThreadValid == false ){
      self->workerThreadValid = true;
#ifdef _WIN32
      THREAD_CREATE(self->workerThread, workerThread, self, self->threadId);
      if(self->workerThread == INVALID_HANDLE_VALUE){
         self->workerThreadValid = false;
         return -1;
      }
#else
      int rc = THREAD_CREATE(self->workerThread, workerThread, self);
      if(rc != 0){
         self->workerThreadValid = false;
         return -1;
      }
#endif
      return 0;
   }
   errno = EINVAL;
   return -1;
}

static void apx_fileManager_stopWorkerThread(apx_fileManager_t *self)
{
   if ( self->workerThreadValid == true )
   {
   #ifdef _MSC_VER
         DWORD result;
   #endif
         apx_msg_t msg = {APX_MSG_EXIT,0,0,0,0}; //{msgType, sender, msgData1, msgData2, msgData3}
         SPINLOCK_ENTER(self->lock);
         adt_rbfs_insert(&self->messages,(const uint8_t*) &msg);
         SPINLOCK_LEAVE(self->lock);
         SEMAPHORE_POST(self->semaphore);
   #ifdef _MSC_VER
         result = WaitForSingleObject(self->workerThread, 5000);
         if (result == WAIT_TIMEOUT)
         {
            APX_LOG_ERROR("[APX_FILE_MANAGER] timeout while joining workerThread");
         }
         else if (result == WAIT_FAILED)
         {
            DWORD lastError = GetLastError();
            APX_LOG_ERROR("[APX_FILE_MANAGER]  joining workerThread failed with %d", (int)lastError);
         }
         CloseHandle(self->workerThread);
         self->workerThread = INVALID_HANDLE_VALUE;
   #else
         if (pthread_equal(pthread_self(), self->workerThread) == 0)
         {
            void *status;
            int s = pthread_join(self->workerThread, &status);
            if (s != 0)
            {
               APX_LOG_ERROR("[APX_FILE_MANAGER] pthread_join error %d\n", s);
            }
         }
         else
         {
            APX_LOG_ERROR("[APX_FILE_MANAGER] pthread_join attempted on pthread_self()\n");
         }
   #endif
   self->workerThreadValid = false;
   }
}

static void apx_fileManager_triggerSendAcknowledge(apx_fileManager_t *self)
{
   apx_msg_t msg = {APX_MSG_SEND_ACKNOWLEDGE, 0, 0, 0, 0};
   SPINLOCK_ENTER(self->lock);
   adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
   SPINLOCK_LEAVE(self->lock);
   SEMAPHORE_POST(self->semaphore);
}

static void apx_fileManager_fileCreatedCbk(void *arg, const struct apx_file_tag *pFile)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->fileCreate != 0)
         {
            listener->fileCreate(listener->arg, self, (struct apx_file_tag*) pFile);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->mutex);
   }
}

static void apx_fileManager_sendFileInfoCbk(void *arg, const struct apx_file_tag *pFile)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if (self != 0)
   {
      apx_msg_t msg = {APX_MSG_SEND_FILEINFO, 0, 0, 0, 0};
      msg.msgData3 = (void*) &pFile->fileInfo;
      SPINLOCK_ENTER(self->lock);
      adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
      SPINLOCK_LEAVE(self->lock);
      SEMAPHORE_POST(self->semaphore);
   }
}

/*** workerThread functions ***/

static THREAD_PROTO(workerThread,arg)
{
   if(arg!=0)
   {
      apx_msg_t msg;
      apx_fileManager_t *self;
      uint32_t messages_processed=0;
      bool isRunning=true;
      self = (apx_fileManager_t*) arg;
      while(isRunning == true)
      {

#ifdef _MSC_VER
         DWORD result = WaitForSingleObject(self->semaphore, INFINITE);
         if (result == WAIT_OBJECT_0)
#else
         int result = sem_wait(&self->semaphore);
         if (result == 0)
#endif
         {
            SPINLOCK_ENTER(self->lock);
            adt_rbfs_remove(&self->messages,(uint8_t*) &msg);
            SPINLOCK_LEAVE(self->lock);
            if (!workerThread_processMessage(self, &msg))
            {
               isRunning = false;
            }
            messages_processed++;
         }
      }
      APX_LOG_INFO("[APX_FILE_MANAGER]: messages_processed: %u",messages_processed);
   }
   THREAD_RETURN(0);
}

static bool workerThread_processMessage(apx_fileManager_t *self, apx_msg_t *msg)
{
   switch(msg->msgType)
   {
   case APX_MSG_EXIT:
      return false;
   case APX_MSG_SEND_FILEINFO:
      workerThread_sendFileInfo(self, msg);
      break;
   case APX_MSG_SEND_ACKNOWLEDGE:
      workerThread_sendAcknowledge(self);
      break;
   default:
      APX_LOG_ERROR("[APX_FILE_MANAGER]: Unknown message type: %u", msg->msgType);
      assert(0);
   }
   return true;
}

static void workerThread_sendFileInfo(apx_fileManager_t *self, apx_msg_t *msg)
{
   int32_t msgSize;
   uint8_t *msgBuf;
   const rmf_fileInfo_t *fileInfo = (const rmf_fileInfo_t*) msg->msgData3;
   assert(fileInfo != 0);
   msgSize = apx_fileManagerShared_calcFileInfoMsgSize(fileInfo);
   assert(self->transmitHandler.getSendBuffer != 0);
   assert(self->transmitHandler.send != 0);
   msgBuf = self->transmitHandler.getSendBuffer(self->transmitHandler.arg, msgSize);
   if (msgBuf != 0)
   {
      int32_t result = apx_fileManagerShared_serializeFileInfo(msgBuf, msgSize, fileInfo);
      if (result > 0)
      {
         self->transmitHandler.send(self->transmitHandler.arg, 0, result);
      }
   }
}

static void workerThread_sendAcknowledge(apx_fileManager_t *self)
{
   int32_t msgSize = RMF_CMD_TYPE_LEN+RMF_ACK_CMD_LEN;
   uint8_t *msgBuf;
   assert(self->transmitHandler.getSendBuffer != 0);
   assert(self->transmitHandler.send != 0);
   msgBuf = self->transmitHandler.getSendBuffer(self->transmitHandler.arg, msgSize);
   if (msgBuf != 0)
   {
      int32_t result = rmf_packHeader(msgBuf, msgSize, RMF_CMD_START_ADDR, false);
      if (result > 0)
      {
         msgBuf+=result;
         result = rmf_serialize_acknowledge(msgBuf, msgSize-result);
         if (result > 0)
         {
            self->transmitHandler.send(self->transmitHandler.arg, 0, msgSize);
         }
      }
   }
}
