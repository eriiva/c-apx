/*****************************************************************************
* \file      apx_nodeDataManager.c
* \author    Conny Gustafsson
* \date      2018-09-03
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
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include "apx_nodeDataManager.h"
#include "apx_node.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_nodeDataManager_mapDestructor(void *arg);
//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_nodeDataManager_create(apx_nodeDataManager_t *self)
{
   if (self != 0)
   {
      apx_istream_handler_t apx_istream_handler;
      memset(&apx_istream_handler,0,sizeof(apx_istream_handler));
      apx_istream_handler.arg = &self->parser;
      apx_istream_handler.open = apx_parser_vopen;
      apx_istream_handler.close = apx_parser_vclose;
      apx_istream_handler.node = apx_parser_vnode;
      apx_istream_handler.datatype = apx_parser_vdatatype;
      apx_istream_handler.provide = apx_parser_vprovide;
      apx_istream_handler.require = apx_parser_vrequire;
      apx_istream_handler.node_end = apx_parser_vnode_end;
      apx_istream_handler.parse_error = apx_parser_vparse_error;
      apx_istream_create(&self->apx_istream, &apx_istream_handler);
      apx_parser_create(&self->parser);
      adt_hash_create(&self->nodeDataMap, apx_nodeDataManager_mapDestructor);
      MUTEX_INIT(self->mutex);
   }
}

void apx_nodeDataManager_destroy(apx_nodeDataManager_t *self)
{
   if (self != 0)
   {
      apx_parser_destroy(&self->parser);
      apx_istream_destroy(&self->apx_istream);
      adt_hash_destroy(&self->nodeDataMap);
      MUTEX_DESTROY(self->mutex);
   }
}

apx_nodeDataManager_t *apx_nodeDataManager_new(void)
{
   apx_nodeDataManager_t *self = (apx_nodeDataManager_t*) malloc(sizeof(apx_nodeDataManager_t));
   if (self != 0)
   {
      apx_nodeDataManager_create(self);
   }
   return self;
}

void apx_nodeDataManager_delete(apx_nodeDataManager_t *self)
{
   if (self != 0)
   {
      apx_nodeDataManager_destroy(self);
      free(self);
   }
}

apx_error_t apx_nodeDataManager_getLastError(apx_nodeDataManager_t *self)
{
   if (self != 0)
   {
      return apx_parser_getLastError(&self->parser);
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

int32_t apx_nodeDataManager_getErrorLine(apx_nodeDataManager_t *self)
{
   if (self != 0)
   {
      return apx_parser_getErrorLine(&self->parser);
   }
   return -1;
}


/**
 * Used to create dynamic nodeData objects by both server and client
 *
 */
apx_nodeData_t *apx_nodeDataManager_createNodeData(apx_nodeDataManager_t *self, const char *name, uint32_t definitionLen)
{
   if ( self != 0)
   {
      return apx_nodeData_new(definitionLen);
   }
   return (apx_nodeData_t*) 0;
}

apx_error_t apx_nodeDataManager_parseNodeDefinition(apx_nodeDataManager_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      apx_error_t lastError;
      if ( (nodeData->definitionDataBuf == 0) || (nodeData->definitionDataLen == 0) )
      {
         return APX_MISSING_BUFFER_ERROR;
      }
      apx_istream_reset(&self->apx_istream);
      apx_istream_open(&self->apx_istream);
      apx_istream_write(&self->apx_istream, nodeData->definitionDataBuf, nodeData->definitionDataLen);
      apx_istream_close(&self->apx_istream);
      lastError = apx_parser_getLastError(&self->parser);
      if (lastError == APX_NO_ERROR)
      {
         int32_t numNodes;
         numNodes = apx_parser_getNumNodes(&self->parser);
         if (numNodes > 1)
         {
            apx_parser_clearNodes(&self->parser);
            return APX_TOO_MANY_NODES_ERROR;
         }
         else if (numNodes == 1)
         {
            apx_node_t *node = apx_parser_getNode(&self->parser, 0);
            apx_nodeData_setNode(nodeData, node);
            apx_parser_clearNodes(&self->parser);
            return APX_NO_ERROR;
         }
         else
         {
            return APX_NODE_MISSING_ERROR;
         }
      }
      return lastError;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

#if 0
apx_error_t apx_nodeDataManager_attachStaticNodeData(apx_nodeDataManager_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      const char *name = apx_nodeData_getName(nodeData);
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_nodeDataManager_mapDestructor(void *arg)
{
   apx_nodeData_t *nodeData = (apx_nodeData_t*) arg;
   if (nodeData->isWeakref == false)
   {
      apx_nodeData_delete(nodeData);
   }
   else
   {
      apx_nodeData_destroy(nodeData);
   }
}

