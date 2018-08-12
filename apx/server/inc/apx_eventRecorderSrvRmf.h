/*****************************************************************************
* \file      apx_eventRecorderSrvRmf.h
* \author    Conny Gustafsson
* \date      2018-08-12
* \brief     Event recorder attached to a single instance of apx_fileManager_t
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
#ifndef APX_EVENT_RECORDER_SRV_RMF_H
#define APX_EVENT_RECORDER_SRV_RMF_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////

#include "apx_eventListener.h"
#include "adt_bytearray.h"
#include <stdbool.h>

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "osmacro.h"


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_fileManager_tag;

typedef struct apx_eventRecorderSrvRmf_tag
{
   struct apx_fileManager_tag *fileManager;
   adt_bytearray_t buffer1;
   adt_bytearray_t buffer2;
   adt_bytearray_t *buf; //uses a double-buffer mechanism, this variable points to either buffer1 or buffer2 depending on which is currently in use
   THREAD_T workerThread;
   SPINLOCK_T lock;
#ifdef _MSC_VER
   unsigned int threadId;
#endif
   bool workerThreadValid;
   int32_t minimumUpdateTime; //resolution: 100ms/bit
}apx_eventRecorderSrvRmf_t;

#define APX_EVENT_RECORDER_RMF_DEFAULT_UPDATE_TIME 10 //resolution: 100 milliseconds per bit, this is used to force write even when buffered isn't yet full

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderSrvRmf_create(apx_eventRecorderSrvRmf_t *self, struct apx_fileManager_tag *fileManager, int32_t minimumUpdateTime);
void apx_eventRecorderSrvRmf_destroy(apx_eventRecorderSrvRmf_t *self);
apx_eventRecorderSrvRmf_t *apx_eventRecorderSrvRmf_new(struct apx_fileManager_tag *fileManager, int32_t minimumUpdateTime);
void apx_eventRecorderSrvRmf_delete(apx_eventRecorderSrvRmf_t *self);
void apx_eventRecorderSrvRmf_vdelete(void *arg);

#endif //APX_EVENT_RECORDER_RMF_H
