/*****************************************************************************
* \file      apx_bytePortMap.c
* \author    Conny Gustafsson
* \date      2018-10-09
* \brief     Byte offset to port id map generator
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
#include "apx_bytePortMap.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_bytePortMap_generate(apx_bytePortMap_t *self, adt_ary_t *portList);

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_bytePortMap_create(apx_bytePortMap_t *self, apx_node_t *node, apx_portType_t portType)
{
   if ( (self != 0) && (node != 0) )
   {
      self->mapData = (apx_portId_t*) 0;
      self->mapLen = 0;
      if (portType == APX_REQUIRE_PORT)
      {
         apx_bytePortMap_generate(self, &node->requirePortList);
      }
      else if (portType == APX_PROVIDE_PORT)
      {
         apx_bytePortMap_generate(self, &node->providePortList);
      }
      else
      {
         //MISRA
      }
   }
}

void apx_bytePortMap_destroy(apx_bytePortMap_t *self)
{
   if ( (self != 0) && (self->mapData != 0) )
   {
      free(self->mapData);
   }
}

apx_portId_t apx_bytePortMap_lookup(apx_bytePortMap_t *self, int32_t offset)
{
   return -1;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_bytePortMap_generate(apx_bytePortMap_t *self, adt_ary_t *portList)
{

}


