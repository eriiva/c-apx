/*****************************************************************************
* \file      apx_nodeDataFactory.h
* \author    Conny Gustafsson
* \date      2018-09-03
* \brief     Factory that is capable of creating new apxNodeData_t objects
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
#ifndef APX_NODE_DATA_FACTORY_H
#define APX_NODE_DATA_FACTORY_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_nodeData.h"
#include "adt_list.h"
#include "apx_parser.h"
#include "apx_stream.h"
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

typedef struct apx_nodeDataFactory_tag
{
   apx_parser_t parser;
   apx_istream_t apx_istream; //helper structure for parser
   MUTEX_T mutex; //locking mechanism
   int8_t lastError;
}apx_nodeDataFactory_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_nodeDataFactory_create(apx_nodeDataFactory_t *self);
void apx_nodeDataFactory_destroy(apx_nodeDataFactory_t *self);
apx_nodeDataFactory_t *apx_nodeDataFactory_new(void);
void apx_nodeDataFactory_delete(apx_nodeDataFactory_t *self);
int8_t apx_nodeDataFactory_getLastError(apx_nodeDataFactory_t *self);
apx_nodeData_t *apx_nodeDataFactory_fromString(apx_nodeDataFactory_t *self, const uint8_t *definitionBuf, uint32_t definitionLen);

#endif //APX_NODE_DATA_FACTORY_H
