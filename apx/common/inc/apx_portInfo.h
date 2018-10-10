/*****************************************************************************
* \file      apx_portInfo.h
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     Collects all useful information about a specific port into a single container
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
#ifndef APX_PORT_INFO_H
#define APX_PORT_INFO_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
struct apx_nodeData_tag;
struct apx_file2_tag;

typedef struct apx_portInfo_tag
{
   struct apx_nodeData_tag *nodedata;
   struct apx_file2_tag *file;
   apx_size_t dataSize; //size of the port data
   apx_offset_t offset; //offset in file
   apx_portId_t portIndex; //the index of the port in nodeData, nodeData->node, nodeData->map
}apx_portInfo_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_portInfo_create(apx_portInfo_t *self, struct apx_nodeData_tag *nodedata, apx_portId_t portIndex, apx_size_t dataSize, struct apx_file2_tag *file, int32_t offset);
apx_portInfo_t *apx_portInfo_new(struct apx_nodeData_tag *nodedata, apx_portId_t portIndex,  apx_size_t dataSize, struct apx_file2_tag *file, int32_t offset);
void apx_portInfo_delete(apx_portInfo_t *self);
void apx_portInfo_vdelete(void *arg);

#endif //APX_PORT_INFO_H
