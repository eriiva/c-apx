#ifndef APX_SERVER_CONNECTION_H
#define APX_SERVER_CONNECTION_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#if defined(_MSC_PLATFORM_TOOLSET) && (_MSC_PLATFORM_TOOLSET<=110)
#include "msc_bool.h"
#else
#include <stdbool.h>
#endif
#include "adt_bytearray.h"
#include "apx_fileManager.h"
#include "apx_nodeManager.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
struct apx_server_tag;
#ifdef UNIT_TEST
#define SOCKET_TYPE struct testsocket_tag
#else
#define SOCKET_TYPE struct msocket_t
#endif
SOCKET_TYPE; //this is a forward declaration of the declared type just above

typedef struct apx_serverConnection_tag
{
   apx_fileManager_t fileManager;
   adt_bytearray_t sendBuffer;
   uint32_t connectionId;
   SOCKET_TYPE *socketObject; //we try to avoid redefining the word "socket" here since socket is an actual OS-defined function
   struct apx_server_tag *server; //apx_serverConnection class does not actively use this pointer, it just stores it for apx_server.
   bool isGreetingParsed;
   int8_t debugMode;
   uint8_t numHeaderMaxLen;
}apx_serverConnection_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_serverConnection_create(apx_serverConnection_t *self, uint32_t connectionId, SOCKET_TYPE *socketObject, struct apx_server_tag *server);
void apx_serverConnection_destroy(apx_serverConnection_t *self);
apx_serverConnection_t *apx_serverConnection_new(uint32_t connectionId, SOCKET_TYPE *socketObject, struct apx_server_tag *server);
void apx_serverConnection_delete(apx_serverConnection_t *self);
void apx_serverConnection_vdelete(void *arg);

void apx_serverConnection_start(apx_serverConnection_t *self);
void apx_serverConnection_setDebugMode(apx_serverConnection_t *self, int8_t debugMode);

int8_t apx_serverConnection_dataReceived(apx_serverConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
apx_fileManager_t *apx_serverConnection_getFileManager(apx_serverConnection_t *self);

#undef SOCKET_TYPE

#endif //APX_SERVER_CONNECTION_H
