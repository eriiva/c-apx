/*****************************************************************************
* \file      apx_nodeByteMap.h
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     Memory map of a nodeData_t object. Only used in server mode
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
#ifndef APX_NODE_PORT_MAP_H
#define APX_NODE_PORT_MAP_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"
#include "adt_ary.h"
#include "apx_nodeData.h"
#include "apx_bytePortMap.h"


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_nodeByteMap_tag
{
   apx_bytePortMap_t *requireBytePortMap; //used only in client mode, maps byte offset back to require port ID
   apx_bytePortMap_t *provideBytePortMap; //used only in server mode, maps byte offset back to provide port ID
}apx_nodeByteMap_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_nodeByteMap_create(apx_nodeByteMap_t *self, apx_nodeData_t *nodeData, uint8_t mode);
void apx_nodeByteMap_destroy(apx_nodeByteMap_t *self);
apx_nodeByteMap_t *apx_nodeByteMap_new(apx_nodeData_t *nodeData, uint8_t mode);
void apx_nodeByteMap_delete(apx_nodeByteMap_t *self);

#endif //APX_NODE_PORT_MAP_H
