
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_server.h"
#include "apx_logging.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_serverInfo_tag
{
   uint8_t addressFamily;
}apx_serverInfo_t;

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_server_create_socket_servers(apx_server_t *self, uint16_t tcpPort);
static void apx_server_destroy_socket_servers(apx_server_t *self);
#ifndef UNIT_TEST
static void apx_server_accept(void *arg,msocket_server_t *srv,msocket_t *msocket);
#endif
static int8_t apx_server_data(void *arg, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
static void apx_server_disconnected(void *arg);
static void apx_server_cleanup_connection(apx_serverConnection_t *connection);

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
      adt_list_create(&self->connections,apx_serverConnection_vdelete);
      self->debugMode = APX_DEBUG_NONE;
      apx_nodeManager_create(&self->nodeManager);
      apx_router_create(&self->router);
      apx_nodeManager_setRouter(&self->nodeManager, &self->router);
      MUTEX_INIT(self->mutex);
      adt_u32Set_create(&self->connectionIdSet);
      self->nextConnectionId = 0u;
   }
}


void apx_server_start(apx_server_t *self)
{
#ifndef UNIT_TEST
   if (self != 0)
   {
      msocket_server_start(&self->tcpServer,0,0,self->tcpPort);
   }
#endif
}

void apx_server_destroy(apx_server_t *self)
{
   if (self != 0)
   {
      //close and delete all open server connections
      adt_list_destroy(&self->connections);
      apx_server_destroy_socket_servers(self);
      apx_nodeManager_destroy(&self->nodeManager);
      apx_router_destroy(&self->router);
      MUTEX_DESTROY(self->mutex);
   }
}

void apx_server_setDebugMode(apx_server_t *self, int8_t debugMode)
{
   if (self != 0)
   {
      self->debugMode = debugMode;
      apx_nodeManager_setDebugMode(&self->nodeManager, debugMode);
      apx_router_setDebugMode(&self->router, debugMode);
   }
}

#ifdef UNIT_TEST
void apx_server_accept_test_socket(apx_server_t *self, testsocket_t *socket)
{
   if (self != 0)
   {
      apx_serverConnection_t *newConnection = apx_serverConnection_new(socket, self);
      if (newConnection != 0)
      {
         msocket_handler_t handlerTable;
         adt_list_insert(&self->connections, newConnection);
         memset(&handlerTable,0,sizeof(handlerTable));
         handlerTable.tcp_data = apx_server_data;
         handlerTable.tcp_disconnected = apx_server_disconnected;
         testsocket_setServerHandler(socket, &handlerTable, newConnection);
         apx_serverConnection_start(newConnection);
      }
   }
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
   msocket_server_create(&self->tcpServer,AF_INET, apx_serverConnection_vdelete);
# ifndef _MSC_VER
   msocket_server_create(&self->localServer,AF_LOCAL, apx_serverConnection_vdelete);
# endif
   serverHandler.tcp_accept = apx_server_accept;
   msocket_server_sethandler(&self->tcpServer,&serverHandler,self);
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

#ifndef UNIT_TEST
static void apx_server_accept(void *arg, msocket_server_t *srv,msocket_t *msocket)
{
   apx_server_t *self = (apx_server_t*) arg;
   if (self != 0)
   {
      apx_serverConnection_t *newConnection = apx_serverConnection_new(msocket, self);

      if (newConnection != 0)
      {
         msocket_handler_t handlerTable;

         //add it to our list of connections. The linked list is used to keep track of all open connections
         adt_list_insert(&self->connections,newConnection);

         //attach our (single) instance of the nodeManager with the connection
         //apx_serverConnection_attachNodeManager()

         //on the new msocket, setup its vtable of handler functions.
         memset(&handlerTable,0,sizeof(handlerTable));
         handlerTable.tcp_data = apx_server_data;
         handlerTable.tcp_disconnected = apx_server_disconnected;
         msocket_sethandler(msocket, &handlerTable, newConnection);

         if (msocket->addressFamily == AF_INET)
         {
            APX_LOG_INFO("[APX_SERVER] New connection (%p) from %s", (void*) newConnection, msocket->tcpInfo.addr);
         }
         else
         {
            APX_LOG_INFO("[APX_SERVER] New connection (%p)", (void*)newConnection);
         }
         if (self->debugMode > APX_DEBUG_NONE)
         {
            apx_serverConnection_setDebugMode(newConnection, self->debugMode);
         }
         //activate internal handler
         apx_serverConnection_start(newConnection);

         //now that the handler is setup, start the internal listening thread in the msocket
         msocket_start_io(msocket);
      }
      else
      {
         APX_LOG_ERROR("[APX_SERVER] %s", "apx_serverConnection_new() returned 0");
      }
   }
}
#endif

static int8_t apx_server_data(void *arg, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   apx_serverConnection_t *clientConnection = (apx_serverConnection_t*) arg;
   return apx_serverConnection_dataReceived(clientConnection, dataBuf, dataLen, parseLen);
}

/**
 * called by msocket worker thread when it detects a disconnect event on the msocket
 */
static void apx_server_disconnected(void *arg)
{
   apx_serverConnection_t *connection = (apx_serverConnection_t*) arg;
   if (connection != 0)
   {
      apx_server_t *server = connection->server;
      MUTEX_LOCK(server->mutex);
      adt_list_remove(&server->connections, connection);
      apx_nodeManager_shutdownFileManager(&server->nodeManager, &connection->fileManager);
      apx_server_cleanup_connection(connection);
      MUTEX_UNLOCK(server->mutex);
   }
}

static void apx_server_cleanup_connection(apx_serverConnection_t *connection)
{
#ifndef UNIT_TEST
   switch (connection->msocket->addressFamily)
   {
      APX_LOG_INFO("[APX_SERVER] Client (%p) disconnected", (void*)connection);
      case AF_INET: //intentional fallthrough
      case AF_INET6:
         msocket_server_cleanup_connection(&connection->server->tcpServer, (void*) connection);
         break;
# ifndef _MSC_VER
      case AF_LOCAL:
         msocket_server_cleanup_connection(&connection->server->localServer, (void*) connection);
         break;
# endif
      default:
         break;
   }
#endif
}

