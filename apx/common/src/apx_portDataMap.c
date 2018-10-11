/*****************************************************************************
* \file      apx_portDataMap.c
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     Global map of all port data elements
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
#include "apx_portDataMap.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void apx_portDataMap_create(apx_portDataMap_t *self)
{
   if (self != 0)
   {
      adt_hash_create(&self->internalMap, apx_portData_vdelete);
      MUTEX_INIT(self->mutex);
   }
}

void apx_portDataMap_destroy(apx_portDataMap_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
      adt_hash_destroy(&self->internalMap);
      MUTEX_UNLOCK(self->mutex);
      MUTEX_DESTROY(self->mutex);
   }
}

apx_portData_t *apx_portDataMap_insert(apx_portDataMap_t *self, const char *portSignature, apx_size_t dataSize)
{
   if ( (self != 0) && (portSignature != 0) && (dataSize > 0) )
   {
      apx_portData_t *portData = apx_portData_new(portSignature, dataSize);
      if (portData != 0)
      {
         adt_hash_set(&self->internalMap, portSignature, 0, portData);
      }
      return portData;
   }
   return (apx_portData_t*) 0;
}

apx_portData_t *apx_portDataMap_find(apx_portDataMap_t *self, const char *portSignature)
{
   if ( (self != 0) && (portSignature != 0) )
   {
      void **ppResult = adt_hash_get(&self->internalMap, portSignature, 0);
      if (ppResult != 0)
      {
         return (apx_portData_t*) *ppResult;
      }
   }
   return (apx_portData_t*) 0;
}

apx_error_t apx_portDataMap_remove(apx_portDataMap_t *self, const char *portSignature)
{
   if ( (self != 0) && (portSignature != 0) )
   {
      void **ppResult = adt_hash_remove(&self->internalMap, portSignature, 0);
      if (ppResult != 0)
      {
         apx_portData_t *portData = (apx_portData_t*) *ppResult;
         apx_portData_delete(portData);
         return APX_NO_ERROR;
      }
      return APX_NOT_FOUND_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_portDataMap_lock(apx_portDataMap_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
   }
}

void apx_portDataMap_unlock(apx_portDataMap_t *self)
{
   if (self != 0)
   {
      MUTEX_UNLOCK(self->mutex);
   }
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////



