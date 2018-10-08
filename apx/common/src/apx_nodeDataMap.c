/*****************************************************************************
* \file      apx_nodeDataMap.c
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     memory map of a nodeData_t object
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
#include "apx_nodeDataMap.h"
#include "apx_error.h"
#include "apx_portInfo.h"
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
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_nodeDataMap_create(apx_nodeDataMap_t *self, apx_nodeData_t *nodeData)
{
   if (self != 0)
   {
      self->nodeData = nodeData;
      adt_ary_create(&self->providePortInfoList, apx_portInfo_vdelete);
      adt_ary_create(&self->requirePortInfoList, apx_portInfo_vdelete);
   }
}

void apx_nodeDataMap_destroy(apx_nodeDataMap_t *self)
{
   if (self != 0)
   {
      adt_ary_destroy(&self->providePortInfoList);
      adt_ary_destroy(&self->requirePortInfoList);
   }
}

apx_nodeDataMap_t *apx_nodeDataMap_new(apx_nodeData_t *nodeData)
{
   apx_nodeDataMap_t *self = (apx_nodeDataMap_t*) malloc(sizeof(apx_nodeDataMap_t));
   if(self != 0)
   {
      apx_nodeDataMap_create(self, nodeData);
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;

}

void apx_nodeDataMap_delete(apx_nodeDataMap_t *self)
{
   if (self != 0)
   {
      apx_nodeDataMap_destroy(self);
      free(self);
   }
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


