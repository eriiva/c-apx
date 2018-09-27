
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_server.h"
#include "apx_logging.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_eventRecorderSrvRmf.h"
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
static apx_serverSocketConnection_t *apx_server_create_new_connection(apx_server_t *self, SOCKET_TYPE *sock);
//static int8_t apx_server_data(void *arg, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
//static void apx_server_disconnected(void *arg);
static void apx_server_cleanup_connection(apx_serverBaseConnection_t *connection);
static uint32_t apx_server_generate_connection_id(apx_server_t *self);
static void apx_server_trigger_connected_event_on_listeners(apx_server_t *self, apx_fileManager_t *fileManager);

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
      adt_list_create(&self->connections, apx_serverBaseConnection_vdelete);
      adt_list_create(&self->connectionEventListeners, (void (*)(void*)) 0);
      self->debugMode = APX_DEBUG_NONE;
      apx_router_create(&self->router);
      MUTEX_INIT(self->lock);
      adt_u32Set_create(&self->connectionIdSet);
      self->nextConnectionId = 0u;
      self->numConnections = 0u;
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
      adt_list_destroy(&self->connectionEventListeners);
      adt_u32Set_destroy(&self->connectionIdSet);
      apx_server_destroy_socket_servers(self);
      apx_router_destroy(&self->router);
      MUTEX_DESTROY(self->lock);
   }
}

void apx_server_setDebugMode(apx_server_t *self, int8_t debugMode)
{
   if (self != 0)
   {
      self->debugMode = debugMode;
      apx_router_setDebugMode(&self->router, debugMode);
   }
}

void apx_server_registerConnectionEventListener(apx_server_t *self, apx_connectionEventListener_t *eventListener)
{
   if (self != 0)
   {
      adt_list_insert_unique(&self->connectionEventListeners, eventListener);
   }
}

#ifdef UNIT_TEST
void apx_server_acceptTestSocket(apx_server_t *self, testsocket_t *socket)
{
   apx_server_accept((void*) self, (struct msocket_server_tag*) 0, socket);
}

apx_serverBaseConnection_t *apx_server_getLastConnection(apx_server_t *self)
{
   if (self != 0)
   {
      if (adt_list_is_empty(&self->connections) == false)
      {
         return (apx_serverBaseConnection_t*) adt_list_last(&self->connections);
      }
   }
   return (apx_serverBaseConnection_t*) 0;
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
      if (self->numConnections < APX_SERVER_MAX_CONCURRENT_CONNECTIONS)
      {
         apx_serverSocketConnection_t *newConnection = apx_server_create_new_connection(self, sock);
         if (newConnection != 0)
         {
            apx_fileManager_t *fileManager;
   #ifndef UNIT_TEST
            if (sock->addressFamily == AF_INET)
            {
               APX_LOG_INFO("[APX_SERVER] New connection (%p) from %s", (void*) newConnection, sock->tcpInfo.addr);
            }
            else
            {
               APX_LOG_INFO("[APX_SERVER] New connection (%p)", (void*)newConnection);
            }
            if (self->debugMode > APX_DEBUG_NONE)
            {
               apx_serverConnection_setDebugMode(newConnection, self->debugMode);
            }
   #endif
            fileManager = apx_serverBaseConnection_getFileManager(&newConnection->base);
            apx_server_trigger_connected_event_on_listeners(self, fileManager);
            apx_serverSocketConnection_start(newConnection);

         }
         else
         {
   #ifndef UNIT_TEST
            APX_LOG_ERROR("[APX_SERVER] %s", "apx_serverConnection_new() returned 0");
   #endif
         }
      }
   }
}


static apx_serverSocketConnection_t *apx_server_create_new_connection(apx_server_t *self, SOCKET_TYPE *sock)
{

   uint32_t connectionId;
   apx_serverSocketConnection_t *newConnection;

   connectionId = apx_server_generate_connection_id(self);

   newConnection = apx_serverSocketConnection_new(connectionId, sock, self);

   if (newConnection != 0)
   {
      adt_list_insert(&self->connections, newConnection);
   }
   return newConnection;
}

#if 0
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
      assert(server->numConnections > 0);
      MUTEX_LOCK(server->lock);
      adt_list_remove(&server->connections, connection);
      server->numConnections--;
      apx_fileManager_stop(&connection->fileManager);
      apx_nodeManager_detachFileManager(&server->nodeManager, &connection->fileManager);
      apx_server_cleanup_connection(connection);
      MUTEX_UNLOCK(server->lock);
   }
}
#endif

static void apx_server_cleanup_connection(apx_serverBaseConnection_t *connection)
{
#ifndef UNIT_TEST
   switch (connection->socketObject->addressFamily)
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

/**
 * Generates a unique connection ID by comparing ID candidates against its internal set data structure
 * This function assumes that APX_SERVER_MAX_CONCURRENT_CONNECTIONS is much less than 2^32-1 and that the caller has previously checked that
 * self->numConnections < APX_SERVER_MAX_CONCURRENT_CONNECTIONS
 */
static uint32_t apx_server_generate_connection_id(apx_server_t *self)
{
   for(;;)
   {
      bool result = adt_u32Set_contains(&self->connectionIdSet, self->nextConnectionId);
      if (result == false)
      {
         adt_u32Set_insert(&self->connectionIdSet, self->nextConnectionId);
         break;
      }
      self->nextConnectionId++;
   }
   return self->nextConnectionId;
}

static void apx_server_trigger_connected_event_on_listeners(apx_server_t *self, apx_fileManager_t *fileManager)
{
   adt_list_elem_t *iter = adt_list_iter_first(&self->connectionEventListeners);
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
