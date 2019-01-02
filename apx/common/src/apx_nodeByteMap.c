/*****************************************************************************
* \file      apx_nodeByteMap.c
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
#include <assert.h>
#include "apx_nodeByteMap.h"
#include "apx_error.h"
#include "apx_file2.h"
#include "apx_portData.h"
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
apx_error_t apx_nodeByteMap_create(apx_nodeByteMap_t *self, apx_nodeData_t *nodeData, uint8_t mode)
{
   if ( (self != 0) && (nodeData != 0) && (nodeData->node != 0) )
   {
      apx_error_t retval = APX_NO_ERROR;
      if (mode == APX_CLIENT_MODE)
      {
         self->provideBytePortMap = (apx_bytePortMap_t*) 0;
         self->requireBytePortMap = apx_bytePortMap_new(nodeData->node, APX_REQUIRE_PORT);
         if (self->requireBytePortMap == 0)
         {
            retval = apx_getLastError();
         }
      }
      else
      {
         self->requireBytePortMap = (apx_bytePortMap_t*) 0;
         self->provideBytePortMap = apx_bytePortMap_new(nodeData->node, APX_PROVIDE_PORT);
         if (self->provideBytePortMap == 0)
         {
            retval = apx_getLastError();
         }
      }
      if (retval == APX_NO_ERROR)
      {

      }
      if (retval == APX_NO_ERROR)
      {

      }
      return retval;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_nodeByteMap_destroy(apx_nodeByteMap_t *self)
{
   if (self != 0)
   {
      if (self->requireBytePortMap != 0)
      {
         apx_bytePortMap_delete(self->requireBytePortMap);
      }
      if (self->provideBytePortMap != 0)
      {
         apx_bytePortMap_delete(self->provideBytePortMap);
      }
   }
}

apx_nodeByteMap_t *apx_nodeByteMap_new(apx_nodeData_t *nodeData, uint8_t mode)
{
   apx_nodeByteMap_t *self = (apx_nodeByteMap_t*) malloc(sizeof(apx_nodeByteMap_t));
   if(self != 0)
   {
      apx_error_t errorType = apx_nodeByteMap_create(self, nodeData, mode);
      if (errorType != APX_NO_ERROR)
      {
         apx_setError(errorType);
         free(self);
         self = (apx_nodeByteMap_t*) 0;
      }
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_nodeByteMap_delete(apx_nodeByteMap_t *self)
{
   if (self != 0)
   {
      apx_nodeByteMap_destroy(self);
      free(self);
   }
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
