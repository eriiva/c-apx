//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "CuTest.h"
#include "pack.h"
#include "apx_clientSocketConnection.h"
#include "apx_client.h"
#include "testsocket_spy.h"
#include "apx_fileManager.h"
#include "numheader.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "osmacro.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define DEFAULT_CONNECTION_ID 0
#define ERROR_MSG_SIZE 150

#define CLIENT_RUN(srv, sock) testsocket_run(sock); apx_client_run(srv); testsocket_run(sock); apx_client_run(srv)


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_clientSocketConnection_create(CuTest* tc);
static void test_apx_clientSocketConnection_transmitHandlerSetup(CuTest* tc);
static void test_apx_clientSocketConnection_sendGreetingOnConnect(CuTest* tc);
static void testsocket_send_acknowledge(testsocket_t *sock);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
static const char *m_TestNodeDefinition = "APX/1.2\n"
      "N\"TestNode\"\n"
      "T\"VehicleSpeed_T\"S\n"
      "R\"VehicleSpeed\"T[0]:=65535\n";


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_clientSocketConnection(void)
{
   CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, test_apx_clientSocketConnection_create);
   SUITE_ADD_TEST(suite, test_apx_clientSocketConnection_transmitHandlerSetup);
   SUITE_ADD_TEST(suite, test_apx_clientSocketConnection_sendGreetingOnConnect);
   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_clientSocketConnection_create(CuTest* tc)
{
   apx_clientSocketConnection_t conn;
   testsocket_t *sock1, *sock2;
   sock1 = testsocket_new(); //apx_clientSocketConnection_t takes ownership of this object. No need to manually delete it
   sock2 = testsocket_new();
   CuAssertIntEquals(tc, 0, apx_clientSocketConnection_create(&conn, sock1, NULL));
   CuAssertUIntEquals(tc, 0, conn.base.base.connectionId);
   CuAssertPtrEquals(tc, sock1, conn.socketObject);
   apx_clientSocketConnection_destroy(&conn);
   CuAssertIntEquals(tc, 0, apx_clientSocketConnection_create(&conn, sock2, NULL));
   CuAssertUIntEquals(tc, 0, conn.base.base.connectionId);
   CuAssertPtrEquals(tc, sock2, conn.socketObject);
   apx_clientSocketConnection_destroy(&conn);
}

static void test_apx_clientSocketConnection_transmitHandlerSetup(CuTest* tc)
{
   apx_clientSocketConnection_t *conn;
   testsocket_t *sock;
   apx_fileManager_t *fileManager;
   testsocket_spy_create();
   sock = testsocket_spy_client();
   conn = apx_clientSocketConnection_new(sock, NULL);
   CuAssertPtrNotNull(tc, conn);
   fileManager = &conn->base.base.fileManager;
   CuAssertPtrNotNull(tc, fileManager->transmitHandler.send);
   apx_clientSocketConnection_delete(conn);
   testsocket_spy_destroy();
}

static void test_apx_clientSocketConnection_sendGreetingOnConnect(CuTest* tc)
{
   apx_client_t *client;
   testsocket_t *sock;
   uint32_t len;
   adt_str_t *str;
   const char *expectedGreeting = "RMFP/1.0\nNumHeader-Format:32\n\n";
   const char *data;
   testsocket_spy_create();
   client = apx_client_new();
   sock = testsocket_spy_server();
   CuAssertPtrNotNull(tc, sock);

   CuAssertIntEquals(tc, 0, testsocket_spy_getServerConnectedCount());
   apx_client_socketConnect(client, sock);
   CLIENT_RUN(client, sock);
   data = (const char*) testsocket_spy_getReceivedData(&len);
   CuAssertIntEquals(tc, 31, testsocket_spy_getServerBytesReceived());
   CuAssertIntEquals(tc, 30, data[0]);
   str = adt_str_make(&data[1], &data[1]+30);
   CuAssertStrEquals(tc, expectedGreeting, adt_str_cstr(str));
   adt_str_delete(str);

   apx_client_delete(client);
   testsocket_spy_destroy();

}

static void testsocket_send_acknowledge(testsocket_t *sock)
{
   uint8_t buffer[10];
   int32_t dataLen;
   int32_t bufSize = (int32_t) sizeof(buffer) - 1;
   dataLen = rmf_packHeader(&buffer[1], bufSize, RMF_CMD_START_ADDR, false);
   dataLen +=rmf_serialize_acknowledge(&buffer[1+dataLen], bufSize - dataLen);
   assert(dataLen == 8);
   buffer[0]=dataLen;
   testsocket_serverSend(sock, &buffer[0], 1+dataLen);
}

