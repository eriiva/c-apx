/*****************************************************************************
* \file      apx_portData.h
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     A port data element contains lists of all provide-ports and require-ports currently associated with a port signature
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
void apx_portData_create(apx_portData_t *self, const char *portSignature, int32_t dataSize)
{
   if (self != 0)
   {
      self->portSignature = portSignature;
      self->dataSize = dataSize;
      adt_ary_create(&self->requirePorts, (void (*)(void*)) 0);
      adt_ary_create(&self->providePorts, (void (*)(void*)) 0);
   }
}

void apx_portData_destroy(apx_portData_t *self)
{
   if (self != 0)
   {
      adt_ary_destroy(&self->requirePorts);
      adt_ary_destroy(&self->providePorts);
   }
}

apx_portData_t *apx_portData_new(const char *portSignature, int32_t dataSize)
{
   apx_portData_t *self = (apx_portData_t*) malloc(sizeof(apx_portData_t));
   if(self != 0)
   {
      apx_portData_create(self, portSignature, dataSize);
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_portData_delete(apx_portData_t *self)
{
   if (self != 0)
   {
      apx_portData_destroy(self);
      free(self);
   }
}

void apx_portData_vdelete(void *arg)
{
   apx_portData_delete((apx_portData_t*) arg);
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


