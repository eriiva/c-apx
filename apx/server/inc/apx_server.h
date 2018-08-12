#ifndef APX_SERVER_H
#define APX_SERVER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_PLATFORM_TOOLSET) && (_MSC_PLATFORM_TOOLSET<=110)
#include "msc_bool.h"
#else
#include <stdbool.h>
#endif
#ifdef UNIT_TEST
#include "testsocket.h"
#else
#include "msocket_server.h"
#endif
#include "apx_nodeManager.h"
#include "apx_serverConnection.h"
#include "apx_router.h"
#include "apx_eventListener.h"
#include "adt_list.h"
#include "adt_set.h"


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//forward declarations


typedef struct apx_server_tag
{
#ifndef UNIT_TEST
   uint16_t tcpPort; //TCP port for tcpServer
   char *localServerFile; //path to socket file for unix domain sockets (used for localServer)
   msocket_server_t tcpServer; //tcp server
   msocket_server_t localServer; //unix domain socket server (to be implemented later)
#endif
   MUTEX_T lock;
   adt_list_t connections; //linked list of strong references to apx_serverConnection_t
   adt_list_t globalEventListeners; //weak references to apx_globalEventListenerBase_t
   apx_nodeManager_t nodeManager; //the server has a single instance of the node manager, all connections interface with this object
   apx_router_t router; //this component handles all routing tables within the server
   int8_t debugMode;
   adt_u32Set_t connectionIdSet;
   uint32_t nextConnectionId;
   uint32_t numConnections;
}apx_server_t;

#define APX_SERVER_MAX_CONCURRENT_CONNECTIONS 10000

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
void apx_server_create(apx_server_t *self);
#else
void apx_server_create(apx_server_t *self, uint16_t port);
#endif
void apx_server_destroy(apx_server_t *self);
void apx_server_start(apx_server_t *self);
void apx_server_setDebugMode(apx_server_t *self, int8_t debugMode);
void apx_server_registerGlobalEventListener(apx_server_t *self, apx_eventListenerBase_t *eventListener);
#ifdef UNIT_TEST
void apx_server_acceptTestSocket(apx_server_t *self, testsocket_t *socket);
apx_serverConnection_t *apx_server_getLastConnection(apx_server_t *self);
#endif


#endif //APX_SERVER_H
