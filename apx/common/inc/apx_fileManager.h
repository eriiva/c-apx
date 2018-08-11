/*****************************************************************************
* \file      apx_fileManager.h
* \author    Conny Gustafsson
* \date      2018-08-04
* \brief     New APX file manager
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
#ifndef APX_FILE_MANAGER_H
#define APX_FILE_MANAGER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_fileManagerDefs.h"
#include "apx_fileManagerShared.h"
#include "apx_fileManagerRemote.h"
#include "apx_fileManagerLocal.h"
#include "apx_transmitHandler.h"
#ifndef ADT_RBFS_ENABLE
#define ADT_RBFS_ENABLE 1
#endif
#include "adt_ringbuf.h"
#ifndef _WIN32
#include <semaphore.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_fileManager_tag
{
   apx_fileManagerShared_t shared;
   apx_fileManagerRemote_t remote;
   apx_fileManagerLocal_t local;
   adt_list_t eventListeners; //contains strong references to apx_eventListener_t
   apx_transmitHandler_t transmitHandler;
   uint8_t mode;
   MUTEX_T mutex;
   SPINLOCK_T lock; //used exclusively by workerThread message queue
   THREAD_T workerThread; //local worker thread
   SEMAPHORE_T semaphore; //thread semaphore
   adt_rbfs_t messages; //pending messages
   bool workerThreadValid;
   uint8_t *ringbufferData; //strong pointer to raw data used by our ringbuffer
   uint32_t ringbufferLen; //number of items in ringbuffer
   bool isConnected;
   int8_t headerSize;

#ifdef _WIN32
   unsigned int threadId;
#endif
}apx_fileManager_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_fileManager_create(apx_fileManager_t *self, uint8_t mode, uint32_t connectionId);
void apx_fileManager_destroy(apx_fileManager_t *self);

void* apx_fileManager_registerEventListener(apx_fileManager_t *self, struct apx_eventListener_tag* listener);
void apx_fileManager_unregisterEventListener(apx_fileManager_t *self, void *handle);
int32_t apx_fileManager_getNumEventListeners(apx_fileManager_t *self);
void apx_fileManager_attachLocalFile(apx_fileManager_t *self, struct apx_file_tag *localFile);
int32_t apx_fileManager_getNumLocalFiles(apx_fileManager_t *self);
bool apx_fileManager_isServerMode(apx_fileManager_t *self);
void apx_fileManager_triggerNewConnectionEvent(apx_fileManager_t *self);
void apx_fileManager_start(apx_fileManager_t *self);
void apx_fileManager_stop(apx_fileManager_t *self);
void apx_fileManager_onHeaderReceived(apx_fileManager_t *self); //used in server mode
void apx_fileManager_onHeaderAccepted(apx_fileManager_t *self); //used in client mode
int32_t apx_fileManager_parseMessage(apx_fileManager_t *self, const uint8_t *msgBuf, int32_t msgLen);
uint32_t fileManager_getID(apx_fileManager_t *self);
void apx_fileManager_setTransmitHandler(apx_fileManager_t *self, apx_transmitHandler_t *handler);


#endif //APX_FILE_MANAGER_H
