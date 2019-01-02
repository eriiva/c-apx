/*****************************************************************************
* \file      apx_portDataRef.h
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
#include "apx_routingTableEntry.h"
#include "apx_nodeData.h"
#include "apx_port.h"
#include "apx_connectionBase.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_routingTableEntry_insertRequirePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData);
static void apx_routingTableEntry_insertProvidePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData);
static void apx_routingTableEntry_removeRequirePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData);
static void apx_routingTableEntry_removeProvidePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData);
static void apx_routingTableEntry_notifyAllRequirePorts(apx_routingTableEntry_t *self, apx_portDataRef_t *providePortDataRef);
static void apx_routingTableEntry_notifyAllProvidePorts(apx_routingTableEntry_t *self, apx_portDataRef_t *requirePortDataRef);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_routingTableEntry_create(apx_routingTableEntry_t *self, const char *portSignature)
{
   if (self != 0)
   {
      self->portSignature = portSignature;
      self->currentProviderId = -1;
      adt_list_create(&self->requirePortRef, (void (*)(void*)) 0);
      adt_list_create(&self->providePortRef, (void (*)(void*)) 0);
   }
}

void apx_routingTableEntry_destroy(apx_routingTableEntry_t *self)
{
   if (self != 0)
   {
      adt_list_destroy(&self->requirePortRef);
      adt_list_destroy(&self->providePortRef);
   }
}

apx_routingTableEntry_t *apx_routingTableEntry_new(const char *portSignature)
{
   apx_routingTableEntry_t *self = (apx_routingTableEntry_t*) malloc(sizeof(apx_routingTableEntry_t));
   if(self != 0)
   {
      apx_routingTableEntry_create(self, portSignature);
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_routingTableEntry_delete(apx_routingTableEntry_t *self)
{
   if (self != 0)
   {
      apx_routingTableEntry_destroy(self);
      free(self);
   }
}

void apx_routingTableEntry_vdelete(void *arg)
{
   apx_routingTableEntry_delete((apx_routingTableEntry_t*) arg);
}

void apx_routingTableEntry_attachPortDataRef(apx_routingTableEntry_t *self, apx_portDataRef_t *portDataRef)
{
   if ( (self != 0) && (portDataRef != 0) )
   {
      if (apx_portDataRef_isProvidePortRef(portDataRef) == true)
      {
         apx_routingTableEntry_insertProvidePortData(self, portDataRef);
         apx_routingTableEntry_notifyAllRequirePorts(self, portDataRef);
      }
      else
      {
         apx_routingTableEntry_insertRequirePortData(self, portDataRef);
         apx_routingTableEntry_notifyAllProvidePorts(self, portDataRef);
      }
   }
}

void apx_routingTableEntry_detachPortDataRef(apx_routingTableEntry_t *self, apx_portDataRef_t *portDataRef)
{
   if ( (self != 0) && (portDataRef != 0) )
   {
      if (apx_portDataRef_isProvidePortRef(portDataRef) == true)
      {
         apx_routingTableEntry_removeProvidePortData(self, portDataRef);
      }
      else
      {
         apx_routingTableEntry_removeRequirePortData(self, portDataRef);
      }
   }
}

bool apx_routingTableEntry_isEmpty(apx_routingTableEntry_t *self)
{
   if (self != 0)
   {
      return (bool) ( (adt_list_is_empty(&self->providePortRef) == true) &&  (adt_list_is_empty(&self->requirePortRef) == true));
   }
   return false;
}



//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_routingTableEntry_insertRequirePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData)
{
   if ((self != 0) && (portData != 0))
   {
      adt_list_insert(&self->requirePortRef, portData);
   }
}

static void apx_routingTableEntry_insertProvidePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData)
{
   if ((self != 0) && (portData != 0))
   {
      adt_list_insert(&self->providePortRef, portData);
   }
}

static void apx_routingTableEntry_removeRequirePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData)
{
   if ((self != 0) && (portData != 0))
   {
      adt_list_remove(&self->requirePortRef, portData);
   }
}

static void apx_routingTableEntry_removeProvidePortData(apx_routingTableEntry_t *self, apx_portDataRef_t *portData)
{
   if ((self != 0) && (portData != 0))
   {
      adt_list_remove(&self->providePortRef, portData);
   }
}

static void apx_routingTableEntry_notifyAllRequirePorts(apx_routingTableEntry_t *self, apx_portDataRef_t *providePortDataRef)
{
   if (adt_list_length(&self->requirePortRef) > 0)
   {
      apx_connectionBase_t *providePortConnection;
      adt_list_elem_t *iter;
      providePortConnection = apx_nodeData_getConnection(providePortDataRef->nodeData);
      for(iter = adt_list_iter_first(&self->requirePortRef); iter != 0; iter = adt_list_iter_next(iter))
      {
         apx_connectionBase_t *requirePortConnection;
         apx_portDataRef_t *requirePortDataRef = (apx_portDataRef_t*) iter->pItem;
         requirePortConnection = apx_nodeData_getConnection(requirePortDataRef->nodeData);
         apx_connectionBase_emitNodePortConnectedEvent(providePortConnection, providePortDataRef, requirePortDataRef);
         apx_connectionBase_emitNodePortConnectedEvent(requirePortConnection, requirePortDataRef, providePortDataRef);
      }
   }
}

static void apx_routingTableEntry_notifyAllProvidePorts(apx_routingTableEntry_t *self, apx_portDataRef_t *requirePortDataRef)
{
   if (adt_list_length(&self->providePortRef) > 0)
   {
      apx_connectionBase_t *requirePortConnection;
      adt_list_elem_t *iter;
      requirePortConnection = apx_nodeData_getConnection(requirePortDataRef->nodeData);
      for(iter = adt_list_iter_first(&self->providePortRef); iter != 0; iter = adt_list_iter_next(iter))
      {
         apx_connectionBase_t *providePortConnection;
         apx_portDataRef_t *providePortDataRef = (apx_portDataRef_t*) iter->pItem;
         providePortConnection = apx_nodeData_getConnection(providePortDataRef->nodeData);
         apx_connectionBase_emitNodePortConnectedEvent(providePortConnection, providePortDataRef, requirePortDataRef);
         apx_connectionBase_emitNodePortConnectedEvent(requirePortConnection, requirePortDataRef, providePortDataRef);
      }
   }
}
