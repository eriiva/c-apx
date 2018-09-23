/*****************************************************************************
* \file      apx_nodeDataManager.h
* \author    Conny Gustafsson
* \date      2018-09-03
* \brief     Dynamically create new apx_nodeData objects
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
#ifndef APX_NODE_DATA_MANAGER_H
#define APX_NODE_DATA_MANAGER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_nodeData.h"
#include "adt_list.h"
#include "apx_parser.h"
#include "apx_stream.h"
#include "apx_error.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "osmacro.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_NODE_DATA_FACTORY_NO_ERROR 0u

typedef struct apx_nodeDataManager_tag
{
   apx_parser_t parser;
   apx_istream_t apx_istream; //helper structure for parser
   MUTEX_T mutex; //locking mechanism
}apx_nodeDataManager_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_nodeDataManager_create(apx_nodeDataManager_t *self);
void apx_nodeDataManager_destroy(apx_nodeDataManager_t *self);
apx_nodeDataManager_t *apx_nodeDataManager_new(void);
void apx_nodeDataManager_delete(apx_nodeDataManager_t *self);
apx_error_t apx_nodeDataManager_getLastError(apx_nodeDataManager_t *self);
int32_t apx_nodeDataManager_getErrorLine(apx_nodeDataManager_t *self);
apx_nodeData_t *apx_nodeDataManager_newNodeData(apx_nodeDataManager_t *self, const char *name);
apx_error_t apx_nodeDataManager_parseDefinition(apx_nodeDataManager_t *self, apx_nodeData_t *nodeData, const uint8_t *definitionBuf, uint32_t definitionLen);


#endif //APX_NODE_DATA_MANAGER_H
