#ifndef APX_CLIENT_H
#define APX_CLIENT_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_clientConnection_tag;
struct apx_nodeManager_tag;
struct apx_nodeData_tag;
struct adt_list_tag;
struct apx_connectionEventListener_tag;
struct apx_fileManager_tag;
#ifdef UNIT_TEST
struct testsocket_tag;
#endif

typedef struct apx_client_tag
{
   struct apx_clientConnection_tag *connection;
   struct apx_nodeManager_tag *nodeManager;
   struct adt_list_tag *eventListeners; //weak references to apx_eventListenerBase_t
}apx_client_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_client_create(apx_client_t *self);
void apx_client_destroy(apx_client_t *self);
apx_client_t *apx_client_new(void);
void apx_client_delete(apx_client_t *self);
void apx_client_vdelete(void *arg);

#ifdef UNIT_TEST
int8_t apx_client_connect(apx_client_t *self, struct testsocket_tag *socketObject);
#else
int8_t apx_client_connectTcp(apx_client_t *self, const char *address, uint16_t port);
# ifndef _WIN32
int8_t apx_client_connectUnix(apx_client_t *self, const char *socketPath);
# endif
#endif
void apx_client_disconnect(apx_client_t *self);
void apx_client_attachLocalNode(apx_client_t *self, struct apx_nodeData_tag *nodeData);
void apx_client_registerEventListener(apx_client_t *self, struct apx_connectionEventListener_tag *eventListener);

//APX internal API
void _apx_client_on_connect(apx_client_t *self, struct apx_fileManager_tag *fileManager);
void _apx_client_on_disconnect(apx_client_t *self, struct apx_fileManager_tag *fileManager);


#endif //APX_CLIENT_H
