/*****************************************************************************
* \file      apx_portInfo.c
* \author    Conny Gustafsson
* \date      2018-10-08
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
void apx_portInfo_create(apx_portInfo_t *self, struct apx_nodeData_tag *nodedata, apx_portId_t portIndex, struct apx_file2_tag *file, int32_t offset)
{
   if (self != 0)
   {
      self->nodedata = nodedata;
      self->file = file;
      self->offset = offset;
      self->portIndex = portIndex;
   }
}

apx_portInfo_t *apx_portInfo_new(struct apx_nodeData_tag *nodedata, apx_portId_t portIndex, struct apx_file2_tag *file, int32_t offset)
{
   apx_portInfo_t *self = (apx_portInfo_t*) malloc(sizeof(apx_portInfo_t));
   if(self != 0)
   {
      apx_portInfo_create(self, nodedata, portIndex, file, offset);
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_portInfo_delete(apx_portInfo_t *self)
{
   if (self != 0)
   {
      free(self);
   }
}

void apx_portInfo_vdelete(void *arg)
{
   apx_portInfo_delete((apx_portInfo_t*) arg);
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


