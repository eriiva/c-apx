/*****************************************************************************
* \file      apx_routingTable.c
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     Global map of all port data elements
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
#include "apx_routingTable.h"
#include "apx_nodeData.h"
#include "apx_node.h"
#include <stdio.h> //DEBUG ONLY

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_routingTable_attachPort(apx_routingTable_t *self, apx_nodeData_t *nodeData, apx_port_t *port);
static void apx_routingTable_detachPort(apx_routingTable_t *self, apx_nodeData_t *nodeData, apx_port_t *port);
static apx_routingTableEntry_t *apx_routingTable_createNewEntry(apx_routingTable_t *self, const char *portSignature);
static apx_portDataRef_t *apx_routingTable_getPortDataRef(apx_nodeData_t *nodeData, apx_port_t *port);

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void apx_routingTable_create(apx_routingTable_t *self)
{
   if (self != 0)
   {
      adt_hash_create(&self->internalMap, apx_routingTableEntry_vdelete);
      MUTEX_INIT(self->mutex);
   }
}

void apx_routingTable_destroy(apx_routingTable_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
      adt_hash_destroy(&self->internalMap);
      MUTEX_UNLOCK(self->mutex);
      MUTEX_DESTROY(self->mutex);
   }
}

apx_routingTableEntry_t *apx_routingTable_insert(apx_routingTable_t *self, const char *portSignature)
{
   if ( (self != 0) && (portSignature != 0) )
   {
      apx_routingTableEntry_t *entry = apx_routingTableEntry_new(portSignature);
      if (entry != 0)
      {
         adt_hash_set(&self->internalMap, portSignature, entry);
      }
      return entry;
   }
   return (apx_routingTableEntry_t*) 0;
}

/**
 * Finds an entry in the routing table. Caller must have locked the resource before making this call by using apx_routingTable_lock
 */
apx_routingTableEntry_t *apx_routingTable_findNoLock(apx_routingTable_t *self, const char *portSignature)
{
   if ( (self != 0) && (portSignature != 0) )
   {
      void **ppResult = adt_hash_get(&self->internalMap, portSignature);
      if (ppResult != 0)
      {
         return (apx_routingTableEntry_t*) *ppResult;
      }
   }
   return (apx_routingTableEntry_t*) 0;
}

/**
 * Identical to apx_routingTable_find but manages locking/unlocking automatically
 */
apx_routingTableEntry_t *apx_routingTable_find(apx_routingTable_t *self, const char *portSignature)
{
   if (self != 0)
   {
      apx_routingTableEntry_t *result;
      apx_routingTable_lock(self);
      result = apx_routingTable_findNoLock(self, portSignature);
      apx_routingTable_unlock(self);
      return result;
   }
   return (apx_routingTableEntry_t*) 0;
}

apx_error_t apx_routingTable_remove(apx_routingTable_t *self, const char *portSignature)
{
   if ( (self != 0) && (portSignature != 0) )
   {
      apx_routingTableEntry_t *entry = (apx_routingTableEntry_t*) adt_hash_remove(&self->internalMap, portSignature);
      if (entry != 0)
      {
         apx_routingTableEntry_delete(entry);
         return APX_NO_ERROR;
      }
      return APX_NOT_FOUND_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_routingTable_lock(apx_routingTable_t *self)
{
   if (self != 0)
   {
      MUTEX_LOCK(self->mutex);
   }
}

void apx_routingTable_unlock(apx_routingTable_t *self)
{
   if (self != 0)
   {
      MUTEX_UNLOCK(self->mutex);
   }
}

int32_t apx_routingTable_length(apx_routingTable_t *self)
{
   if (self != 0)
   {
      return adt_hash_length(&self->internalMap);
   }
   return -1;
}

void apx_routingTable_attachNodeData(apx_routingTable_t *self, struct apx_nodeData_tag *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      apx_node_t *node = apx_nodeData_getNode(nodeData);
      if (node != 0)
      {
         int32_t numRequirePorts;
         int32_t numProvidePorts;
         int32_t portId;
         numRequirePorts = apx_node_getNumRequirePorts(node);
         numProvidePorts = apx_node_getNumProvidePorts(node);
         apx_routingTable_lock(self);
         for (portId = 0; portId<numRequirePorts; portId++)
         {
            apx_port_t *port = apx_node_getRequirePort(node, portId);
            apx_routingTable_attachPort(self, nodeData, port);
         }
         for (portId = 0; portId<numProvidePorts; portId++)
         {
            apx_port_t *port = apx_node_getProvidePort(node, portId);
            apx_routingTable_attachPort(self, nodeData, port);
         }
         apx_routingTable_unlock(self);
      }
   }
}

void apx_routingTable_detachNodeData(apx_routingTable_t *self, struct apx_nodeData_tag *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      apx_node_t *node = apx_nodeData_getNode(nodeData);
      if (node != 0)
      {
         int32_t numRequirePorts;
         int32_t numProvidePorts;
         int32_t portId;
         numRequirePorts = apx_node_getNumRequirePorts(node);
         numProvidePorts = apx_node_getNumProvidePorts(node);
         apx_routingTable_lock(self);
         for (portId = 0; portId<numRequirePorts; portId++)
         {
            apx_port_t *port = apx_node_getRequirePort(node, portId);
            apx_routingTable_detachPort(self, nodeData, port);
         }
         for (portId = 0; portId<numProvidePorts; portId++)
         {
            apx_port_t *port = apx_node_getProvidePort(node, portId);
            apx_routingTable_detachPort(self, nodeData, port);
         }
         apx_routingTable_unlock(self);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_routingTable_attachPort(apx_routingTable_t *self, apx_nodeData_t *nodeData, apx_port_t *port)
{
   if (port != 0)
   {
      apx_routingTableEntry_t *entry;
      apx_portDataRef_t *portDataRef;
      const char *portSignature = apx_port_getDerivedPortSignature(port);
      entry = apx_routingTable_findNoLock(self, portSignature);
      if (entry == 0)
      {
         entry = apx_routingTable_createNewEntry(self, portSignature);
      }
      if (entry != 0)
      {
         portDataRef = apx_routingTable_getPortDataRef(nodeData, port);
         apx_routingTableEntry_attachPortDataRef(entry, portDataRef);
      }
   }
}

static void apx_routingTable_detachPort(apx_routingTable_t *self, apx_nodeData_t *nodeData, apx_port_t *port)
{
   if (port != 0)
   {
      apx_routingTableEntry_t *entry;
      apx_portDataRef_t *portDataRef;
      const char *portSignature = apx_port_getDerivedPortSignature(port);
      entry = apx_routingTable_findNoLock(self, portSignature);
      if (entry != 0)
      {
         portDataRef = apx_routingTable_getPortDataRef(nodeData, port);
         apx_routingTableEntry_detachPortDataRef(entry, portDataRef);
         if (apx_routingTableEntry_isEmpty(entry) == true)
         {
            apx_routingTable_remove(self, portSignature);
         }
      }
   }
}

static apx_routingTableEntry_t *apx_routingTable_createNewEntry(apx_routingTable_t *self, const char *portSignature)
{
   apx_routingTableEntry_t *entry = apx_routingTableEntry_new(portSignature);
   if (entry != 0)
   {
      adt_hash_set(&self->internalMap, portSignature, entry);
   }
   return entry;
}

static apx_portDataRef_t *apx_routingTable_getPortDataRef(apx_nodeData_t *nodeData, apx_port_t *port)
{
   apx_portDataRef_t *retval = (apx_portDataRef_t*) 0;
   if (port->portType == APX_REQUIRE_PORT)
   {
      retval = apx_nodeData_getRequirePortDataRef(nodeData, port->portId);
   }
   else
   {
      retval = apx_nodeData_getProvidePortDataRef(nodeData, port->portId);
   }
   return retval;
}

