/*****************************************************************************
* \file      apx_serverEventRecorder.c
* \author    Conny Gustafsson
* \date      2018-05-01
* \brief     Records APX events into its internal pendingBuffer and occasionally sends them out to a logging object
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
#include <errno.h>
#include "apx_eventRecorderSrvBin.h"
#include <assert.h>
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
#if 0
void apx_remoteFileLog_create(apx_remoteFileLog_t *self, struct apx_fileManager_tag *manager)
{
   if ( (self != 0) && (manager != 0) )
   {
      apx_file_t *apx_file;
      rmf_fileInfo_t info;
      self->manager = manager;
      rmf_fileInfo_create(&info, APX_EVENT_SRV_FILE_NAME, APX_RMF_EVENT_FILE_ADDRESS, APX_EVENT_FILE_LEN, RMF_FILE_TYPE_STREAM);
      apx_file = apx_file_new(APX_EVENT_FILE, &info);
      if (apx_file != 0)
      {
#if 0
         apx_fileManager_attachLocalDataFile(parent, apx_file); //note that file manager takes ownership of apx_file and will be responsible for deleting it.
#endif
      }
   }
}

void apx_remoteFileLog_destroy(apx_remoteFileLog_t *self)
{
   if (self != 0)
   {
   }
}

apx_remoteFileLog_t *apx_remoteFileLog_new(struct apx_fileManager_tag *parent)
{
   apx_remoteFileLog_t *self = (apx_remoteFileLog_t*) malloc(sizeof(apx_remoteFileLog_t));
   if(self != 0)
   {
      apx_remoteFileLog_create(self, parent);
   }
   return self;
}

void apx_remoteFileLog_delete(apx_remoteFileLog_t *self)
{
   if(self != 0)
   {
      apx_remoteFileLog_destroy(self);
      free(self);
   }
}
#endif
void apx_serverEventRecorder_create(apx_serverEventRecorder_t *self, uint32_t maxPendSize)
{
   if (self != 0)
   {
      adt_bytearray_create(&self->pendingBuffer, ADT_BYTE_ARRAY_DEFAULT_GROW_SIZE);
      self->maxPendSize = maxPendSize;
   }
}

void apx_serverEventRecorder_destroy(apx_serverEventRecorder_t *self)
{
   if (self != 0)
   {
      adt_bytearray_destroy(&self->pendingBuffer);
   }
}

apx_serverEventRecorder_t *apx_serverEventRecorder_new(uint32_t maxPendSize)
{
   apx_serverEventRecorder_t *self = (apx_serverEventRecorder_t*) malloc(sizeof(apx_serverEventRecorder_t));
   if(self != 0)
   {
      apx_serverEventRecorder_create(self, maxPendSize);
   }
   return self;
}

void apx_serverEventRecorder_delete(apx_serverEventRecorder_t *self)
{
   if(self != 0)
   {
      apx_serverEventRecorder_destroy(self);
      free(self);
   }
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


