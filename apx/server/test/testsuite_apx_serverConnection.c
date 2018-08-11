//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdio.h>
#include "CuTest.h"
#include "apx_serverConnection.h"
#include "apx_server.h"
#include "testsocketspy.h"
#include "apx_fileManager.h"
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

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_serverConnection_create(CuTest* tc);
static void test_apx_serverConnection_transmitHandlerSetup(CuTest* tc);
static void test_apx_serverConnection_sendAckAfterReceivingHeader(CuTest* tc);
static void sendHeader(testsocket_t *sock);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_serverConnection(void)
{
   CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, test_apx_serverConnection_create);
   SUITE_ADD_TEST(suite, test_apx_serverConnection_transmitHandlerSetup);
   SUITE_ADD_TEST(suite, test_apx_serverConnection_sendAckAfterReceivingHeader);
   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_serverConnection_create(CuTest* tc)
{
   apx_serverConnection_t conn;
   testsocket_t *sock1, *sock2;
   sock1 = testsocket_new(); //apx_serverConnection_t takes ownership of this object. No need to manually delete it
   sock2 = testsocket_new();
   CuAssertIntEquals(tc, 0, apx_serverConnection_create(&conn, 0, sock1, NULL));
   CuAssertUIntEquals(tc, 0, conn.connectionId);
   CuAssertPtrEquals(tc, sock1, conn.socketObject);
   apx_serverConnection_destroy(&conn);
   CuAssertIntEquals(tc, 0, apx_serverConnection_create(&conn, 1, sock2, NULL));
   CuAssertUIntEquals(tc, 1, conn.connectionId);
   CuAssertPtrEquals(tc, sock2, conn.socketObject);
   apx_serverConnection_destroy(&conn);
}

static void test_apx_serverConnection_transmitHandlerSetup(CuTest* tc)
{
   apx_serverConnection_t *conn;
   testsocket_t *sock;
   apx_fileManager_t *fileManager;
   testsocket_spy_create();
   sock = testsocket_spy_client();
   conn = apx_serverConnection_new(DEFAULT_CONNECTION_ID, sock, NULL);
   fileManager = &conn->fileManager;
   CuAssertPtrNotNull(tc, fileManager->transmitHandler.send);
   apx_serverConnection_delete(conn);
   testsocket_spy_destroy();
}

static void test_apx_serverConnection_sendAckAfterReceivingHeader(CuTest* tc)
{
   apx_server_t server;
   testsocket_t *sock;
   uint32_t len;
   const uint8_t *data;
   const uint8_t expected[9] = {8, 0xBF, 0xFF, 0xFC, 0x00, 0, 0, 0, 0};
   int32_t i;
   testsocket_spy_create();
   sock = testsocket_spy_client();
   apx_server_create(&server);
   apx_server_start(&server);
   apx_server_accept_test_socket(&server, sock);
   testsocket_onConnect(sock);
   sendHeader(sock);
   testsocket_run(sock);
   SLEEP(50);
   testsocket_run(sock);
   data = testsocket_spy_getReceivedData(&len);
   CuAssertUIntEquals(tc, RMF_CMD_TYPE_LEN+RMF_ACK_CMD_LEN+1, len);
   for(i=0;i<(int32_t) sizeof(expected);i++)
   {
      char msg[14];
      sprintf(msg, "i=%d",i);
      CuAssertIntEquals_Msg(tc, msg, expected[i], data[i]);
   }
   apx_server_destroy(&server);
   testsocket_spy_destroy();
}

static void sendHeader(testsocket_t *sock)
{
   const char *greeting = "RMFP/1.0\nNumHeader-Format:32\n\n";
   int32_t msgLen;
   uint8_t msg[RMF_GREETING_MAX_LEN];
   msgLen = (int32_t) strlen(greeting);
   msg[0] = (uint8_t) msgLen;
   memcpy(&msg[1], greeting, msgLen);
   testsocket_clientSend(sock, &msg[0], 1+msgLen);
}

