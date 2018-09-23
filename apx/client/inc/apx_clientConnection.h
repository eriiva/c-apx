#ifndef APX_CLIENT_CONNECTION_H
#define APX_CLIENT_CONNECTION_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include "adt_bytearray.h"
#include "apx_fileManager.h"
#include "apx_error.h"
#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
#else
# include <pthread.h>
#endif
#include "osmacro.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_client_tag;

#ifdef UNIT_TEST
#define SOCKET_TYPE struct testsocket_tag
#else
#define SOCKET_TYPE struct msocket_t
#endif
SOCKET_TYPE; //this is a forward declaration


/**
 * APX client-side connection
 */
typedef struct apx_clientConnection_tag
{
   struct apx_fileManager_tag fileManager;
   SOCKET_TYPE *socketObject;
   SPINLOCK_T lock;
   bool isConnected;
   bool isAcknowledgeSeen;
   adt_bytearray_t sendBuffer;
   uint8_t maxMsgHeaderSize;
   struct apx_client_tag *client;
}apx_clientConnection_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_clientConnection_create(apx_clientConnection_t *self, struct apx_client_tag *client);
void apx_clientConnection_destroy(apx_clientConnection_t *self);
apx_clientConnection_t *apx_clientConnection_new(struct apx_client_tag *client);
void apx_clientConnection_delete(apx_clientConnection_t *self);
void apx_clientConnection_vdelete(void *arg);
#ifdef UNIT_TEST
int8_t apx_clientConnection_connect(apx_clientConnection_t *self, SOCKET_TYPE *socketObject);
#else
int8_t apx_clientConnection_connectTcp(apx_clientConnection_t *self, const char *address, uint16_t port);
# ifndef _WIN32
apx_error_t apx_clientConnection_connectUnix(apx_clientConnection_t *self, const char *socketPath);
# endif
#endif
void apx_clientConnection_disconnect(apx_clientConnection_t *self);

#undef SOCKET_TYPE

#endif //APX_CLIENT_CONNECTION_H
