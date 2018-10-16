/*****************************************************************************
* \file      apx_eventLoop.c
* \author    Conny Gustafsson
* \date      2018-10-15
* \brief     APX event loop
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
#include <malloc.h>
#include <assert.h>
#include "apx_eventLoop.h"
#include "apx_event.h"
#include "apx_logging.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST
static THREAD_PROTO(workerThread,arg);
#endif
static apx_error_t apx_eventLoop_startWorkerThread(apx_eventLoop_t *self);
static void apx_eventLoop_stopWorkerThread(apx_eventLoop_t *self);
static void workerThread_processEvent(apx_eventLoop_t *self, apx_event_t *event);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_eventLoop_create(apx_eventLoop_t *self)
{
   if (self != 0)
   {
      size_t numItems = APX_MAX_NUM_MESSAGES;
      size_t elemSize = APX_EVENT_SIZE;
      self->ringbufferLen = numItems;
      self->ringbufferData = (uint8_t*) malloc(numItems*elemSize);

      if (self->ringbufferData == 0)
      {
         return APX_MEM_ERROR;
      }
      adt_rbfs_create(&self->pendingEvents, self->ringbufferData,(uint16_t) numItems,(uint8_t) elemSize);
      SPINLOCK_INIT(self->lock);
      SEMAPHORE_CREATE(self->semaphore);
   #ifdef _WIN32
      self->workerThread = INVALID_HANDLE_VALUE;
   #else
      self->workerThread = 0;
   #endif
      self->workerThreadValid=false;
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_eventLoop_destroy(apx_eventLoop_t *self)
{
   if (self != 0)
   {
      if (self->workerThreadValid == true)
      {
         apx_eventLoop_stop(self);
      }
      SPINLOCK_DESTROY(self->lock);
      if (self->ringbufferData != 0)
      {
         free(self->ringbufferData);
      }
   }
}

apx_eventLoop_t *apx_eventLoop_new(void)
{
   apx_eventLoop_t *self = (apx_eventLoop_t*) malloc(sizeof(apx_eventLoop_t));
   if (self != 0)
   {
      apx_error_t errorType = apx_eventLoop_create(self);
      if (errorType != APX_NO_ERROR)
      {
         apx_setError(errorType);
         free(self);
         self = (apx_eventLoop_t*) 0;
      }
   }
   return self;
}

void apx_eventLoop_delete(apx_eventLoop_t *self)
{
   if (self != 0)
   {
      apx_eventLoop_destroy(self);
      free(self);
   }
}

void apx_eventLoop_start(apx_eventLoop_t *self)
{
   if (self != 0)
   {
      apx_eventLoop_startWorkerThread(self);
   }
}

void apx_eventLoop_stop(apx_eventLoop_t *self)
{
   if (self != 0)
   {
      apx_eventLoop_stopWorkerThread(self);
   }
}

void apx_eventLoop_emitConnected(apx_eventLoop_t *self, apx_eventListener_connectedFn_t *handler, void *arg, struct apx_fileManager_tag *fileManager)
{
   if (self != 0)
   {
      apx_event_t event = {APX_EVENT_CONNECTED, 0, 0, 0, 0, 0};
      event.handler = (void*) handler;
      event.evData1 = arg;
      event.evData2 = (void*) fileManager;
      SPINLOCK_ENTER(self->lock);
      adt_rbfs_insert(&self->pendingEvents, (const uint8_t*) &event);
      SPINLOCK_LEAVE(self->lock);
   }
}




//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static apx_error_t apx_eventLoop_startWorkerThread(apx_eventLoop_t *self)
{
#ifndef UNIT_TEST
   if( self->workerThreadValid == false ){
      self->workerThreadValid = true;
      self->workerThreadRun = true;
#ifdef _WIN32
      THREAD_CREATE(self->workerThread, workerThread, self, self->threadId);
      if(self->workerThread == INVALID_HANDLE_VALUE){
         self->workerThreadValid = false;
         return APX_THREAD_CREATE_ERROR;
      }
#else
      int rc = THREAD_CREATE(self->workerThread, workerThread, self);
      if(rc != 0){
         self->workerThreadValid = false;
         self->workerThreadRun = false;
         return APX_THREAD_CREATE_ERROR;
      }
#endif
   }
#else
   self->workerThreadRun = true;
#endif
   return APX_NO_ERROR;
}

static void apx_eventLoop_stopWorkerThread(apx_eventLoop_t *self)
{
   if ( self->workerThreadValid == true )
   {
   #ifdef _MSC_VER
         DWORD result;
   #endif
         SPINLOCK_ENTER(self->lock);
         self->workerThreadRun = false;
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

#ifndef UNIT_TEST
static THREAD_PROTO(workerThread,arg)
{
   if(arg!=0)
   {
      apx_event_t event;
      apx_eventLoop_t *self;
      bool isRunning=true;
      self = (apx_eventLoop_t*) arg;
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
            isRunning = self->workerThreadRun;
            if (isRunning == true)
            {
               adt_rbfs_remove(&self->pendingEvents,(uint8_t*) &event);
            }
            SPINLOCK_LEAVE(self->lock);
            if (isRunning == true)
            {
               workerThread_processEvent(self, &event);
            }
         }
      }
   }
   THREAD_RETURN(0);
}
#endif

#ifdef UNIT_TEST
void apx_eventLoop_run(apx_eventLoop_t *self)
{
   apx_event_t event;
   bool isRunning = self->workerThreadRun;
   while(isRunning == true)
   {
      uint8_t rc = adt_rbfs_remove(&self->pendingEvents,(uint8_t*) &event);
      if (rc == E_BUF_OK)
      {
         workerThread_processEvent(self, &event);
      }
      else
      {
         break;
      }
      isRunning = self->workerThreadRun;
   }
}
#endif

static void workerThread_processEvent(apx_eventLoop_t *self, apx_event_t *event)
{
   union handler_tag
   {
      apx_eventListener_connectedFn_t *connected;
      apx_eventListener_disconnectedFn_t *disconnected;
   }handler;
   switch(event->evType)
   {
   case APX_EVENT_CONNECTED:
      handler.connected = (apx_eventListener_connectedFn_t*) event->handler;
      handler.connected(event->evData1, (struct apx_fileManager_tag*)event->evData2);
      break;
   default:
      assert(0);
   }
}
