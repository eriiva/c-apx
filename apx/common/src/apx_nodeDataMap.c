/*****************************************************************************
* \file      apx_nodeDataMap.c
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
#include "apx_nodeDataMap.h"
#include "apx_error.h"
#include "apx_file2.h"
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
static apx_error_t apx_nodeDataMap_createProvidePortInfo(apx_nodeDataMap_t *self);
static apx_error_t apx_nodeDataMap_createRequirePortInfo(apx_nodeDataMap_t *self);
static apx_error_t apx_nodeDataMap_generatePortInfoList(adt_ary_t *portInfoList, apx_nodeData_t *nodeData, adt_ary_t *portList, apx_file2_t *file);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_nodeDataMap_create(apx_nodeDataMap_t *self, apx_nodeData_t *nodeData, uint8_t mode)
{
   if ( (self != 0) && (nodeData != 0) && (nodeData->node != 0) )
   {
      apx_error_t retval = APX_NO_ERROR;
      self->nodeData = nodeData;
      if (mode == APX_CLIENT_MODE)
      {
         self->provideBytePortMap = (apx_bytePortMap_t*) 0;
         self->requireBytePortMap = apx_bytePortMap_new(self->nodeData->node, APX_REQUIRE_PORT);
         if (self->requireBytePortMap == 0)
         {
            retval = apx_getLastError();
         }
      }
      else
      {
         self->requireBytePortMap = (apx_bytePortMap_t*) 0;
         self->provideBytePortMap = apx_bytePortMap_new(self->nodeData->node, APX_PROVIDE_PORT);
         if (self->provideBytePortMap == 0)
         {
            retval = apx_getLastError();
         }
      }
      if (retval == APX_NO_ERROR)
      {
         retval = apx_nodeDataMap_createProvidePortInfo(self);
      }
      if (retval == APX_NO_ERROR)
      {
         apx_nodeDataMap_createRequirePortInfo(self);
      }
      return retval;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_nodeDataMap_destroy(apx_nodeDataMap_t *self)
{
   if (self != 0)
   {
      adt_ary_destroy(&self->providePortInfoList);
      adt_ary_destroy(&self->requirePortInfoList);
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

apx_nodeDataMap_t *apx_nodeDataMap_new(apx_nodeData_t *nodeData, uint8_t mode)
{
   apx_nodeDataMap_t *self = (apx_nodeDataMap_t*) malloc(sizeof(apx_nodeDataMap_t));
   if(self != 0)
   {
      apx_error_t errorType = apx_nodeDataMap_create(self, nodeData, mode);
      if (errorType != APX_NO_ERROR)
      {
         apx_setError(errorType);
         free(self);
         self = (apx_nodeDataMap_t*) 0;
      }
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_nodeDataMap_delete(apx_nodeDataMap_t *self)
{
   if (self != 0)
   {
      apx_nodeDataMap_destroy(self);
      free(self);
   }
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static apx_error_t apx_nodeDataMap_createProvidePortInfo(apx_nodeDataMap_t *self)
{
   adt_ary_t *portList = &self->nodeData->node->providePortList;
   apx_file2_t *file = apx_nodeData_getOutPortDataFile(self->nodeData);
   adt_ary_create(&self->providePortInfoList, apx_portInfo_vdelete);
   return apx_nodeDataMap_generatePortInfoList(&self->providePortInfoList, self->nodeData, portList, file);
}

static apx_error_t apx_nodeDataMap_createRequirePortInfo(apx_nodeDataMap_t *self)
{
   adt_ary_t *portList = &self->nodeData->node->requirePortList;
   apx_file2_t *file = apx_nodeData_getInPortDataFile(self->nodeData);
   adt_ary_create(&self->requirePortInfoList, apx_portInfo_vdelete);
   return apx_nodeDataMap_generatePortInfoList(&self->requirePortInfoList, self->nodeData, portList, file);
}

static apx_error_t apx_nodeDataMap_generatePortInfoList(adt_ary_t *portInfoList, apx_nodeData_t *nodeData, adt_ary_t *portList, apx_file2_t *file)
{
   apx_offset_t offset = 0;
   apx_portId_t portIndex;
   int32_t numPorts;
   numPorts = adt_ary_length(portList);
   for (portIndex=0;portIndex<numPorts;portIndex++)
   {
      apx_portInfo_t *portInfo;
      apx_size_t dataSize;
      apx_port_t *port = (apx_port_t*) adt_ary_value(portList, portIndex);
      assert(port != 0);
      dataSize = apx_port_getPackLen(port);
      if (dataSize <= 0)
      {
         return APX_LENGTH_ERROR;
      }
      portInfo = apx_portInfo_new(nodeData, portIndex, dataSize, file, offset);
      if (portInfo == 0)
      {
         return APX_MEM_ERROR;
      }
      adt_ary_push(portInfoList, portInfo);
      offset += dataSize;
   }
   return APX_NO_ERROR;
}
