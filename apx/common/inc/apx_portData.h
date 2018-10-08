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
#ifndef APX_PORT_DATA_H
#define APX_PORT_DATA_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "adt_ary.h"
#include "apx_types.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_portData_tag
{
   const char *portSignature; //weak reference to string
   int32_t dataSize;
   adt_ary_t requirePorts; //weak references to apx_portInfo_t
   adt_ary_t providePorts; //weak references to apx_portInfo_t
}apx_portData_t;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_portData_create(apx_portData_t *self, const char *portSignature, int32_t dataSize);
void apx_portData_destroy(apx_portData_t *self);
apx_portData_t *apx_portData_new(const char *portSignature, int32_t dataSize);
void apx_portData_delete(apx_portData_t *self);
void apx_portData_vdelete(void *arg);

#endif //APX_PORT_DATA_H
