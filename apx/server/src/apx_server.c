
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_server.h"
#include "apx_logging.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_serverSocketConnection.h"
#include <assert.h>
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_serverInfo_tag
{
   uint8_t addressFamily;
}apx_serverInfo_t;

struct msocket_server_tag;

#ifdef UNIT_TEST
#define SOCKET_TYPE testsocket_t
#define SOCKET_DELETE testsocket_delete
#define SOCKET_START_IO(x)
#define SOCKET_SET_HANDLER testsocket_setServerHandler
#else
#define SOCKET_DELETE msocket_delete
#define SOCKET_TYPE msocket_t
#define SOCKET_START_IO(x) msocket_start_io(x)
#define SOCKET_SET_HANDLER msocket_sethandler
#endif

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_server_create_socket_servers(apx_server_t *self, uint16_t tcpPort);
static void apx_server_destroy_socket_servers(apx_server_t *self);
static void apx_server_accept(void *arg, struct msocket_server_tag *srv, SOCKET_TYPE *sock);
static void apx_server_attach_and_start_connection(apx_server_t *self, apx_serverConnectionBase_t *newConnection);
static void apx_server_trigger_connected_event_on_listeners(apx_server_t *self, apx_connectionBase_t *connection);


//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
void apx_server_create(apx_server_t *self)
#else
void apx_server_create(apx_server_t *self, uint16_t port)
#endif
{
   if (self != 0)
   {
#ifndef UNIT_TEST
      apx_server_create_socket_servers(self, port);
#else
      apx_server_create_socket_servers(self, 0);
#endif
      adt_list_create(&self->connectionEventListeners, (void (*)(void*)) 0);
      self->debugMode = APX_DEBUG_NONE;
      apx_routingTable_create(&self->routingTable);
      apx_connectionManager_create(&self->connectionManager);
   }
}


void apx_server_start(apx_server_t *self)
{
#ifndef UNIT_TEST
   if (self != 0)
   {
      apx_connectionManager_start(&self->connectionManager);
      msocket_server_start(&self->tcpServer, 0, 0, self->tcpPort);
   }
#endif
}

void apx_server_destroy(apx_server_t *self)
{
   if (self != 0)
   {
      //close and delete all open server connections
      adt_list_destroy(&self->connectionEventListeners);
      apx_connectionManager_stop(&self->connectionManager);
      apx_connectionManager_destroy(&self->connectionManager);
      apx_server_destroy_socket_servers(self);
      apx_routingTable_destroy(&self->routingTable);
   }
}

void apx_server_setDebugMode(apx_server_t *self, int8_t debugMode)
{
   if (self != 0)
   {
      self->debugMode = debugMode;
      //apx_router_setDebugMode(&self->router, debugMode);
   }
}

void apx_server_registerConnectionEventListener(apx_server_t *self, apx_connectionEventListener_t *eventListener)
{
   if (self != 0)
   {
      adt_list_insert_unique(&self->connectionEventListeners, eventListener);
   }
}

void apx_server_acceptConnection(apx_server_t *self, apx_serverConnectionBase_t *serverConnection)
{
   if ( (self != 0) && (serverConnection != 0))
   {
      apx_server_attach_and_start_connection(self, serverConnection);
   }
}

void apx_server_removeConnection(apx_server_t *self, apx_serverConnectionBase_t *serverConnection)
{
   if ( (self != 0) && (serverConnection != 0))
   {
      apx_connectionManager_shutdown(&self->connectionManager, serverConnection);
   }
}

apx_routingTable_t* apx_server_getRoutingTable(apx_server_t *self)
{
   if (self != 0)
   {
      return &self->routingTable;
   }
   return (apx_routingTable_t*) 0;
}

#ifdef UNIT_TEST

void apx_server_run(apx_server_t *self)
{
   if (self != 0)
   {
      apx_connectionManager_run(&self->connectionManager);
   }
}


void apx_server_acceptTestSocket(apx_server_t *self, testsocket_t *socket)
{
   apx_server_accept((void*) self, (struct msocket_server_tag*) 0, socket);
}

apx_serverConnectionBase_t *apx_server_getLastConnection(apx_server_t *self)
{
   if (self != 0)
   {
      return apx_connectionManager_getLastConnection(&self->connectionManager);
   }
   return (apx_serverConnectionBase_t*) 0;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void apx_server_create_socket_servers(apx_server_t *self, uint16_t tcpPort)
{
#ifndef UNIT_TEST
   msocket_handler_t serverHandler;
   self->tcpPort = tcpPort;
   memset(&serverHandler,0,sizeof(serverHandler));
   msocket_server_create(&self->tcpServer, AF_INET, apx_serverConnection_vdelete);
# ifndef _MSC_VER
   msocket_server_create(&self->localServer, AF_LOCAL, apx_serverConnection_vdelete);
# endif
   serverHandler.tcp_accept = apx_server_accept;
   msocket_server_sethandler(&self->tcpServer, &serverHandler, self);
#endif
}

static void apx_server_destroy_socket_servers(apx_server_t *self)
{
#ifndef UNIT_TEST
   //destroy the tcp server
   msocket_server_destroy(&self->tcpServer);
   //destroy the local socket server
# ifndef _MSC_VER
   msocket_server_destroy(&self->localServer);
# endif
#endif
}

static void apx_server_accept(void *arg, struct msocket_server_tag *srv, SOCKET_TYPE *sock)
{
   apx_server_t *self = (apx_server_t*) arg;
   if (self != 0)
   {
      if (apx_connectionManager_getNumConnections(&self->connectionManager) < APX_SERVER_MAX_CONCURRENT_CONNECTIONS)
      {
         apx_serverSocketConnection_t *newConnection = apx_serverSocketConnection_new(sock, self);
         if (newConnection != 0)
         {
            apx_server_attach_and_start_connection(self, (apx_serverConnectionBase_t*) newConnection);
         }
      }
   }
}


static void apx_server_attach_and_start_connection(apx_server_t *self, apx_serverConnectionBase_t *newConnection)
{
   apx_connectionManager_attach(&self->connectionManager, newConnection);
   apx_server_trigger_connected_event_on_listeners(self, (apx_connectionBase_t*) newConnection);
   apx_connectionBase_start(&newConnection->base);
}

static void apx_server_trigger_connected_event_on_listeners(apx_server_t *self, apx_connectionBase_t *connection)
{
   adt_list_elem_t *iter = adt_list_iter_first(&self->connectionEventListeners);
   while(iter != 0)
   {
      apx_connectionEventListener_t *listener = (apx_connectionEventListener_t*) iter->pItem;
      if ( (listener != 0) && (listener->connected != 0))
      {
         listener->connected(listener, connection);
      }
      iter = adt_list_iter_next(iter);
   }
}
