//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "apx_client.h"
#include "apx_clientConnection.h"
#include "apx_nodeManager.h"
#include "apx_fileManager.h"
#include "msocket.h"
#include "adt_list.h"
#include "apx_nodeData.h"
#include "apx_eventListener.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif



//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_client_triggerConnectedEventOnListeners(apx_client_t *self, apx_fileManager_t *fileManager);
static void apx_client_triggerDisconnectedEventOnListeners(apx_client_t *self, apx_fileManager_t *fileManager);
//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_client_create(apx_client_t *self)
{
   if( self != 0 )
   {
      self->nodeManager = apx_nodeManager_new();
      if (self->nodeManager == 0)
      {
         return -1;
      }
      self->connection = apx_clientConnection_new(self);
      if (self->connection == 0)
      {
         apx_nodeManager_delete(self->nodeManager);
         return -1;
      }
      self->eventListeners = adt_list_new((void (*)(void*)) 0);
      if (self->eventListeners == 0)
      {
         apx_nodeManager_delete(self->nodeManager);
         apx_clientConnection_delete(self->connection);
         return -1;
      }
      return 0;
   }
   errno=EINVAL;
   return -1;
}

void apx_client_destroy(apx_client_t *self)
{
   if (self != 0)
   {
      apx_clientConnection_delete(self->connection);
      apx_nodeManager_delete(self->nodeManager);
      adt_list_delete(self->eventListeners);
   }
}

apx_client_t *apx_client_new(void)
{
   apx_client_t *self = (apx_client_t*) malloc(sizeof(apx_client_t));
   if(self != 0)
   {
      int8_t result = apx_client_create(self);
      if (result != 0)
      {
         free(self);
         self=0;
      }
   }
   else
   {
      errno = ENOMEM;
   }
   return self;
}

void apx_client_delete(apx_client_t *self)
{
   if (self != 0)
   {
      apx_client_destroy(self);
      free(self);
   }
}

void apx_client_vdelete(void *arg)
{
   apx_client_delete((apx_client_t*) arg);
}

#ifdef UNIT_TEST
int8_t apx_client_connect(apx_client_t *self, struct testsocket_tag *socketObject)
{
   if (self != 0)
   {
      return apx_clientConnection_connect(self->connection, socketObject);
   }
   return -1;
}
#else
int8_t apx_client_connect_tcp(apx_client_t *self, const char *address, uint16_t port)
{
   if (self != 0)
   {
      return apx_clientConnection_connectTcp(self->connection, address, port);
   }
   return -1;
}
#endif

void apx_client_disconnect(apx_client_t *self)
{
   if (self != 0)
   {
      apx_clientConnection_disconnect(self->connection);
   }
}

/**
 * attached the nodeData to the local nodeManager in the client
 */
void apx_client_attach_local_node(apx_client_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      apx_nodeManager_attachLocalNode(self->nodeManager, nodeData);
   }
}

void apx_client_registerEventListener(apx_client_t *self, struct apx_connectionEventListener_tag *eventListener)
{
   if (self != 0)
   {
      adt_list_insert_unique(self->eventListeners, eventListener);
   }
}

void _apx_client_on_connect(apx_client_t *self, struct apx_fileManager_tag *fileManager)
{
   if ( (self != 0) && (fileManager != 0) )
   {
      apx_client_triggerConnectedEventOnListeners(self, fileManager);
   }
}

void _apx_client_on_disconnect(apx_client_t *self, struct apx_fileManager_tag *fileManager)
{
   if ( (self != 0) && (fileManager != 0) )
   {
      apx_client_triggerDisconnectedEventOnListeners(self, fileManager);
   }
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_client_triggerConnectedEventOnListeners(apx_client_t *self, apx_fileManager_t *fileManager)
{
   adt_list_elem_t *iter = adt_list_iter_first(self->eventListeners);
   while(iter != 0)
   {
      apx_connectionEventListener_t *listener = (apx_connectionEventListener_t*) iter->pItem;
      if ( (listener != 0) && (listener->connected != 0))
      {
         listener->connected(listener, fileManager);
      }
      iter = adt_list_iter_next(iter);
   }
}

static void apx_client_triggerDisconnectedEventOnListeners(apx_client_t *self, apx_fileManager_t *fileManager)
{
   adt_list_elem_t *iter = adt_list_iter_first(self->eventListeners);
   while(iter != 0)
   {
      apx_connectionEventListener_t *listener = (apx_connectionEventListener_t*) iter->pItem;
      if ( (listener != 0) && (listener->disconnected != 0))
      {
         listener->disconnected(listener, fileManager);
      }
      iter = adt_list_iter_next(iter);
   }
}
