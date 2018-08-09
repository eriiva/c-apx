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
#include "apx_fileManager.h"
#include "apx_eventListener.h"

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_fileManager_fileCreatedByRemote(void *arg, const struct apx_file_tag *pFile);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_fileManager_create(apx_fileManager_t *self, uint8_t mode, uint32_t connectionId)
{
   int8_t result;
   result = apx_fileManagerShared_create(&self->shared, connectionId);
   if (result == 0)
   {
      apx_fileManagerRemote_create(&self->remote, &self->shared);
      adt_list_create(&self->eventListeners, apx_eventListener_vdelete);
      self->mode = mode;
      self->shared.arg = self;
      self->shared.fileCreatedByRemote = apx_fileManager_fileCreatedByRemote;
   }
   return result;
}

void apx_fileManager_destroy(apx_fileManager_t *self)
{
   if (self != 0)
   {
      adt_list_destroy(&self->eventListeners);
      apx_fileManagerRemote_destroy(&self->remote);
      apx_fileManagerShared_destroy(&self->shared);
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

int32_t apx_fileManager_parseMessage(apx_fileManager_t *self, const uint8_t *msgBuf, int32_t msgLen)
{
   if (self != 0)
   {
      return apx_fileManagerRemote_parseMessage(&self->remote, msgBuf, msgLen);
   }
   return -1;
}

void apx_fileManager_start(apx_fileManager_t *self)
{
   if (self != 0)
   {
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->fileManagerStart != 0)
         {
            listener->fileManagerStart(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }

   }
}

/**
 * called by connection after it has successfully parsed the RMF header
 */
void apx_fileManager_onHeaderAccepted(apx_fileManager_t *self)
{
   if (self != 0)
   {
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->headerAccepted != 0)
         {
            listener->headerAccepted(listener->arg, self);
         }
         pIter = adt_list_iter_next(pIter);
      }
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

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_fileManager_fileCreatedByRemote(void *arg, const struct apx_file_tag *pFile)
{
   apx_fileManager_t *self = (apx_fileManager_t *) arg;
   if (self != 0)
   {
      adt_list_elem_t *pIter = adt_list_iter_first(&self->eventListeners);
      while (pIter != 0)
      {
         apx_eventListener_t *listener = (apx_eventListener_t*) pIter->pItem;
         assert(listener != 0);
         if (listener->fileCreate != 0)
         {
            listener->fileCreate(listener->arg, self, pFile);
         }
         pIter = adt_list_iter_next(pIter);
      }
   }
}

