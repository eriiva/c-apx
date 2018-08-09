/*****************************************************************************
* \file      apx_clientNodeManager.c
* \author    Conny Gustafsson
* \date      2018-08-09
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
#include <malloc.h>
#include "apx_nodeData.h"
#include "apx_clientNodeManager.h"

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

void apx_clientNodeManager_create(apx_clientNodeManager_t *self)
{
   if (self != 0)
   {
      adt_hash_create(&self->localNodeDataMap, (void(*)(void*)) 0);
      adt_list_create(&self->fileManagerList, (void(*)(void*)) 0);
      MUTEX_INIT(self->mutex);
   }
}

void apx_clientNodeManager_destroy(apx_clientNodeManager_t *self)
{
   if (self != 0)
   {
      adt_hash_destroy(&self->localNodeDataMap);
      adt_list_destroy(&self->fileManagerList);
      MUTEX_DESTROY(self->mutex);
   }
}

apx_clientNodeManager_t *apx_clientNodeManager_new(void)
{
   apx_clientNodeManager_t *self = (apx_clientNodeManager_t*) malloc(sizeof(apx_clientNodeManager_t));
   if(self != 0)
   {
      apx_clientNodeManager_create(self);
   }
   return self;
}

void apx_clientNodeManager_delete(apx_clientNodeManager_t *self)
{
   if(self != 0)
   {
      apx_clientNodeManager_destroy(self);
      free(self);
   }
}

void apx_clientNodeManager_attachLocalNode(apx_clientNodeManager_t *self,  struct apx_nodeData_tag *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      adt_hash_set(&self->localNodeDataMap, nodeData->name, 0, nodeData);
   }
}

void apx_clientNodeManager_attachFileManager(apx_clientNodeManager_t *self, struct apx_fileManager_tag *fileManager)
{

}

void apx_clientNodeManager_detachFileManager(apx_clientNodeManager_t *self, struct apx_fileManager_tag *fileManager)
{

}

struct apx_nodeData_tag *apx_clientNodeManager_findLocalNode(apx_clientNodeManager_t *self, const char *name)
{
   if (self != 0)
   {
      void **ptr = adt_hash_get(&self->localNodeDataMap, name, 0);
      if (ptr != 0)
      {
         return (struct apx_nodeData_tag*) *ptr;
      }
   }
   return (struct apx_nodeData_tag*) 0;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


