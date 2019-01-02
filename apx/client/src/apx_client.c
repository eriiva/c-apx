//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "apx_client.h"
#include "apx_clientSocketConnection.h"
#include "apx_nodeDataManager.h"
#include "apx_fileManager.h"
#include "msocket.h"
#include "adt_list.h"
#include "apx_nodeData.h"
#include "apx_eventListener.h"
#ifdef UNIT_TEST
#include "testsocket.h"
#endif
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
apx_error_t apx_client_create(apx_client_t *self)
{
   if( self != 0 )
   {
      self->connection = (apx_clientConnectionBase_t*) 0;
      self->eventListeners = adt_list_new((void (*)(void*)) 0);
      if (self->eventListeners == 0)
      {
         return APX_MEM_ERROR;
      }
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_client_destroy(apx_client_t *self)
{
   if (self != 0)
   {
      if (self->connection != 0)
      {
         apx_connectionBase_delete(&self->connection->base);
      }
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
      apx_setError(APX_MEM_ERROR);
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
apx_error_t apx_client_socketConnect(apx_client_t *self, struct testsocket_tag *socketObject)
{
   if (self != 0)
   {
      apx_clientSocketConnection_t *socketConnection = apx_clientSocketConnection_new(socketObject, self);
      if (socketConnection)
      {
         apx_error_t result;
         self->connection = (apx_clientConnectionBase_t*) socketConnection;
         result = apx_clientSocketConnection_connect(socketConnection);
         if (result == APX_NO_ERROR)
         {
            testsocket_onConnect(socketObject);
         }
         return result;
      }
   }
   return APX_INVALID_ARGUMENT_ERROR;
}
#else
apx_error_t apx_client_tcpConnect(apx_client_t *self, const char *address, uint16_t port)
{
   if (self != 0)
   {
      return apx_clientConnection_connectTcp(self->connection, address, port);
   }
   return -1;
}

# ifndef _WIN32
apx_error_t apx_client_unixConnect(apx_client_t *self, const char *socketPath)
{
   if (self != 0)
   {
      return apx_clientConnection_connectUnix(self->connection, socketPath);
   }
   return -1;
}
# endif

#endif

void apx_client_disconnect(apx_client_t *self)
{
   if ( (self != 0) && (self->connection != 0))
   {
      apx_connectionBase_close(&self->connection->base);
   }
}

/**
 * attached the nodeData to the local nodeManager in the client
 */
void apx_client_attachLocalNode(apx_client_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      //apx_nodeManager_attachLocalNode(self->nodeManager, nodeData);
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

#ifdef UNIT_TEST

#define APX_CLIENT_RUN_CYCLES 10

void apx_client_run(apx_client_t *self)
{
   if (self!=0 && (self->connection != 0))
   {
      int32_t i;
      for(i=0;i<APX_CLIENT_RUN_CYCLES;i++)
      {
         apx_clientConnectionBase_run(self->connection);
      }
   }
}
#endif



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
         listener->connected(listener, (apx_connectionBase_t*) self->connection);
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
         listener->disconnected(listener, (apx_connectionBase_t*) self->connection);
      }
      iter = adt_list_iter_next(iter);
   }
}
