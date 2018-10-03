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
#include "apx_file2.h"
#include "pack.h"


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_fileManager_triggerHeaderReceivedEvent(apx_fileManager_t *self);
static void apx_fileManager_triggerStoppedEvent(apx_fileManager_t *self);
static int8_t apx_fileManager_startWorkerThread(apx_fileManager_t *self);
static void apx_fileManager_stopWorkerThread(apx_fileManager_t *self);
static void apx_fileManager_triggerSendAcknowledge(apx_fileManager_t *self);

static void apx_fileManager_fileCreatedCbk(void *arg, const struct apx_file2_tag *pFile);
static void apx_fileManager_sendFileInfoCbk(void *arg, const struct apx_file2_tag *pFile);
static void apx_fileManager_sendFileOpen(void *arg, const apx_file2_t *file, void *caller);
static void apx_fileManager_openFileRequest(void *arg, uint32_t address);
static void apx_fileManager_processOpenFixedFile(apx_fileManager_t *self, apx_file2_t *localFile);
static void apx_fileManager_triggerFileOpenEvent(apx_fileManager_t *self, const apx_file2_t *file, void *caller);
static void apx_fileManager_sendFixedFile(apx_fileManager_t *self, const apx_file2_t *file);
static void apx_fileManager_sendInvalidReadHandler(apx_fileManager_t *self, uint32_t address);

static THREAD_PROTO(workerThread,arg);
static bool workerThread_processMessage(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendAcknowledge(apx_fileManager_t *self);
static void workerThread_sendFileInfo(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendFileOpen(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendInvalidReadHandler(apx_fileManager_t *self, apx_msg_t *msg);
static void workerThread_sendApxErrorCode(apx_fileManager_t *self, uint32_t errorCode);
static void workerThread_readFile(apx_fileManager_t *self, apx_msg_t *msg);
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
         adt_list_create(&self->eventListeners, apx_fileManagerEventListener_vdelete);
         self->mode = mode;
         self->shared.arg = self;
         self->shared.fileCreated = apx_fileManager_fileCreatedCbk;
         self->shared.sendFileInfo = apx_fileManager_sendFileInfoCbk;
         self->shared.sendFileOpen = apx_fileManager_sendFileOpen;
         self->shared.openFileRequest = apx_fileManager_openFileRequest;
         MUTEX_INIT(self->mutex);
         MUTEX_INIT(self->eventListenerMutex);
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
      MUTEX_DESTROY(self->eventListenerMutex);
      SPINLOCK_DESTROY(self->lock);
      if (self->ringbufferData != 0)
      {
         free(self->ringbufferData);
      }
   }
}

void* apx_fileManager_registerEventListener(apx_fileManager_t *self, struct apx_fileManagerEventListener_tag* listener)
{
   if ( (self != 0) && (listener != 0))
   {
      void *handle = (void*) apx_fileManagerEventListener_clone(listener);
      if (handle != 0)
      {
         MUTEX_LOCK(self->eventListenerMutex);
         adt_list_insert(&self->eventListeners, handle);
         MUTEX_UNLOCK(self->eventListenerMutex);
      }
      return handle;
   }
   return (void*) 0;
}

void apx_fileManager_unregisterEventListener(apx_fileManager_t *self, void *handle)
{
   if ( (self != 0) && (handle != 0))
   {
      MUTEX_LOCK(self->eventListenerMutex);
      bool isFound = adt_list_remove(&self->eventListeners, handle);
      if (isFound == true)
      {
         apx_fileManagerEventListener_vdelete(handle);
      }
      MUTEX_UNLOCK(self->eventListenerMutex);
   }
}

int32_t apx_fileManager_getNumEventListeners(apx_fileManager_t *self)
{
   if (self != 0)
   {
      int32_t result;
      MUTEX_LOCK(self->eventListenerMutex);
      result = adt_list_length(&self->eventListeners);
      MUTEX_UNLOCK(self->eventListenerMutex);
      return result;
   }
   errno = EINVAL;
   return -1;
}

void apx_fileManager_attachLocalFile(apx_fileManager_t *self, struct apx_file2_tag *localFile)
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

int32_t apx_fileManager_processMessage(apx_fileManager_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   if (self != 0)
   {
      return apx_fileManagerRemote_processMessage(&self->remote, msgBuf, msgLen);
   }
   return -1;
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
      apx_fileManager_triggerStoppedEvent(self);
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

/**
 * opens a remote file if it has been previously published by remote side.
 * The caller argument is used to prevent event listener callback to be triggered by this event
 */
int8_t apx_fileManager_openRemoteFile(apx_fileManager_t *self, uint32_t address, void *caller)
{
   if (self != 0)
   {
      return apx_fileManageRemote_openFile(&self->remote, address, caller);
   }
   errno = EINVAL;
   return -1;
}

void apx_fileManager_sendFileAlreadyExistsError(apx_fileManager_t *self, apx_file2_t *file)
{
   if (self != 0)
   {
      apx_msg_t msg = {APX_MSG_ERROR_FILE_ALREADY_EXISTS, 0, 0, 0, 0};
      msg.msgData1 = file->fileInfo.address;
      msg.msgData3 = STRDUP(file->fileInfo.name);
      SPINLOCK_ENTER(self->lock);
      adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
      SPINLOCK_LEAVE(self->lock);
      SEMAPHORE_POST(self->semaphore);
   }
}

struct apx_file2_tag *apx_fileManager_findLocalFileByName(apx_fileManager_t *self, const char *name)
{
   if (self != 0)
   {
      return apx_fileManagerLocal_findByName(&self->local, name);
   }
   return (struct apx_file2_tag *) 0;
}

void apx_fileManager_sendApxErrorCode(apx_fileManager_t *self, uint32_t errorCode)
{
   if ( (self != 0) && (errorCode > RMF_USER_ERROR_BEGIN) )
   {
      apx_msg_t msg = {APX_MSG_ERROR_CODE, 0, 0, 0, 0};
      msg.msgData1 = errorCode;
      SPINLOCK_ENTER(self->lock);
      adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
      SPINLOCK_LEAVE(self->lock);
      SEMAPHORE_POST(self->semaphore);
   }
}


#ifdef UNIT_TEST
bool apx_fileManager_run(apx_fileManager_t *self)
{
   if (self != 0)
   {
      if (adt_rbfs_size(&self->messages) > 0)
      {
         apx_msg_t msg;
         adt_rbfs_remove(&self->messages,(uint8_t*) &msg);
         return workerThread_processMessage(self, &msg);
      }
   }
   return false;
}

int32_t apx_fileManager_numPendingMessages(apx_fileManager_t *self)
{
   if (self != 0)
   {
      return (int32_t) adt_rbfs_size(&self->messages);
   }
   return -1;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void apx_fileManager_triggerHeaderReceivedEvent(apx_fileManager_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->eventListenerMutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_fileManagerEventListener_t *listener = (apx_fileManagerEventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->headerReceived != 0)
         {
            listener->headerReceived(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->eventListenerMutex);
   }
}

static void apx_fileManager_triggerStoppedEvent(apx_fileManager_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->eventListenerMutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_fileManagerEventListener_t *listener = (apx_fileManagerEventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->fileManagerStop != 0)
         {
            listener->fileManagerStop(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->eventListenerMutex);
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

static void apx_fileManager_fileCreatedCbk(void *arg, const struct apx_file2_tag *pFile)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if (self != 0)
   {
      MUTEX_LOCK(self->eventListenerMutex);
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_fileManagerEventListener_t *listener = (apx_fileManagerEventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->fileCreate != 0)
         {
            listener->fileCreate(listener->arg, self, (struct apx_file2_tag*) pFile);
         }
         pIter = adt_list_iter_next(pIter);
      }
      MUTEX_UNLOCK(self->eventListenerMutex);
   }
}

static void apx_fileManager_sendFileInfoCbk(void *arg, const struct apx_file2_tag *pFile)
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

/**
 * Attempts to open someone elses remote file by sending a SEND_FILE_OPEN message
 */
static void apx_fileManager_sendFileOpen(void *arg, const apx_file2_t *file, void *caller)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if ( (self != 0) && (file != 0) )
   {
      apx_msg_t msg = {APX_MSG_SEND_FILE_OPEN, 0, 0, 0, 0};
      msg.msgData1 = file->fileInfo.address;
      SPINLOCK_ENTER(self->lock);
      adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
      SPINLOCK_LEAVE(self->lock);
      SEMAPHORE_POST(self->semaphore);
      apx_fileManager_triggerFileOpenEvent(self, file, caller);
   }
}

/**
 * Attempts to open a local file (by request of remote)
 */
static void apx_fileManager_openFileRequest(void *arg, uint32_t address)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if (self != 0)
   {
      apx_file2_t *localFile = apx_fileManagerLocal_find(&self->local, address);
      if (localFile != 0)
      {
         if (localFile->fileInfo.fileType == RMF_FILE_TYPE_FIXED)
         {
            apx_fileManager_processOpenFixedFile(self, localFile);
         }
         else if (localFile->fileInfo.fileType == RMF_FILE_TYPE_STREAM)
         {
            //TODO: implement
         }
      }
   }
}

static void apx_fileManager_processOpenFixedFile(apx_fileManager_t *self, apx_file2_t *localFile)
{
   if (apx_file2_hasReadHandler(localFile) == true)
   {
      apx_file2_open(localFile);
      apx_fileManager_sendFixedFile(self, localFile);
      apx_fileManager_triggerFileOpenEvent(self, localFile, NULL);
   }
   else
   {
      apx_fileManager_sendInvalidReadHandler(self, localFile->fileInfo.address);
   }
}

static void apx_fileManager_triggerFileOpenEvent(apx_fileManager_t *self, const apx_file2_t *file, void *caller)
{
   adt_list_elem_t *pIter;
   MUTEX_LOCK(self->mutex);
   pIter = adt_list_iter_first(&self->eventListeners);
   MUTEX_UNLOCK(self->eventListenerMutex);
   while (pIter != 0)
   {
      apx_fileManagerEventListener_t *listener = (apx_fileManagerEventListener_t*) pIter->pItem;
      assert(listener != 0);
      if ( (caller != listener) && (listener->fileOpen != 0) )
      {
         listener->fileOpen(listener->arg, self, file);
      }
      pIter = adt_list_iter_next(pIter);
   }
   MUTEX_UNLOCK(self->eventListenerMutex);
}

static void apx_fileManager_sendFixedFile(apx_fileManager_t *self, const apx_file2_t *file)
{
   apx_msg_t msg = {APX_MSG_READ_FILE, 0, 0, 0, 0};
   msg.msgData1 = 0;
   msg.msgData2 = file->fileInfo.length;
   msg.msgData3 = (void*) file;
   SPINLOCK_ENTER(self->lock);
   adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
   SPINLOCK_LEAVE(self->lock);
   SEMAPHORE_POST(self->semaphore);
}

static void apx_fileManager_sendInvalidReadHandler(apx_fileManager_t *self, uint32_t address)
{
   apx_msg_t msg = {APX_MSG_ERROR_INVALID_READ_HANDLER, 0, 0, 0, 0};
   msg.msgData1 = address;
   SPINLOCK_ENTER(self->lock);
   adt_rbfs_insert(&self->messages, (const uint8_t*) &msg);
   SPINLOCK_LEAVE(self->lock);
   SEMAPHORE_POST(self->semaphore);
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
   case APX_MSG_SEND_FILE_OPEN:
      workerThread_sendFileOpen(self, msg);
      break;
   case APX_MSG_SEND_FILE_CLOSE:
      break;
   case APX_MSG_READ_FILE:
      workerThread_readFile(self, msg);
      break;
   case APX_MSG_ERROR_CODE:
      workerThread_sendApxErrorCode(self, msg->msgData1);
      break;
   case APX_MSG_ERROR_INVALID_CMD:
      break;
   case APX_MSG_ERROR_INVALID_WRITE:
      break;
   case APX_MSG_ERROR_INVALID_READ_HANDLER:
      workerThread_sendInvalidReadHandler(self, msg);
      break;
   default:
      APX_LOG_ERROR("[APX_FILE_MANAGER]: Unknown message type: %u", msg->msgType);
      assert(0);
   }
   return true;
}

static void workerThread_sendAcknowledge(apx_fileManager_t *self)
{
   int32_t msgSize = RMF_CMD_HEADER_LEN+RMF_CMD_ACK_LEN;
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

static void workerThread_sendFileOpen(apx_fileManager_t *self, apx_msg_t *msg)
{
   int32_t msgSize = RMF_CMD_HEADER_LEN+RMF_CMD_FILE_OPEN_LEN;
   uint8_t *msgBuf;
   assert(self->transmitHandler.getSendBuffer != 0);
   assert(self->transmitHandler.send != 0);
   msgBuf = self->transmitHandler.getSendBuffer(self->transmitHandler.arg, msgSize);
   if (msgBuf != 0)
   {
      int32_t result = rmf_packHeader(msgBuf, msgSize, RMF_CMD_START_ADDR, false);
      if (result > 0)
      {
         rmf_cmdOpenFile_t cmd;
         cmd.address = msg->msgData1;
         msgBuf+=result;
         result = rmf_serialize_cmdOpenFile(msgBuf, msgSize-result, &cmd);
         if (result > 0)
         {
            self->transmitHandler.send(self->transmitHandler.arg, 0, msgSize);
         }
      }
   }
}

static void workerThread_sendInvalidReadHandler(apx_fileManager_t *self, apx_msg_t *msg)
{
   int32_t msgSize = RMF_CMD_HEADER_LEN+RMF_ERROR_INVALID_READ_HANDLER_LEN;
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
         result = rmf_serialize_errorInvalidReadHandler(msgBuf, msgSize-result, msg->msgData1);
         if (result > 0)
         {
            self->transmitHandler.send(self->transmitHandler.arg, 0, msgSize);
         }
      }
   }
}

static void workerThread_sendApxErrorCode(apx_fileManager_t *self, uint32_t errorCode)
{
   int32_t msgSize = RMF_CMD_HEADER_LEN+RMF_ERROR_CODE_BASE_LEN;
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
         packLE(&msgBuf[0], errorCode, (uint8_t) sizeof(errorCode));
         if (result > 0)
         {
            self->transmitHandler.send(self->transmitHandler.arg, 0, msgSize);
         }
      }
   }
}

static void workerThread_readFile(apx_fileManager_t *self, apx_msg_t *msg)
{
   uint32_t offset = msg->msgData1;
   uint32_t dataLen = msg->msgData2;
   apx_file2_t *file = (apx_file2_t*) msg->msgData3;

   if (file != 0)
   {
      uint32_t startAddress = file->fileInfo.address+offset;
      uint8_t *msgBuf;
      int32_t msgLen;
      int32_t bufLen = (startAddress >= RMF_DATA_HIGH_MIN_ADDR)? RMF_HIGH_ADDRESS_SIZE : RMF_LOW_ADDRESS_SIZE;
      assert(offset+dataLen<=file->fileInfo.length);
      bufLen+=dataLen;
      msgBuf = self->transmitHandler.getSendBuffer(self->transmitHandler.arg, bufLen);
      if (msgBuf != 0)
      {
         msgLen = rmf_packHeader(msgBuf, bufLen, startAddress, false);
         if (msgLen > 0)
         {
            int8_t result = apx_file2_read(file, &msgBuf[msgLen], offset, dataLen);
            if (result == 0)
            {
               self->transmitHandler.send(self->transmitHandler.arg, 0, bufLen);
            }
            else
            {
               APX_LOG_ERROR("APX File read error\n");
            }
         }
      }
   }
}
