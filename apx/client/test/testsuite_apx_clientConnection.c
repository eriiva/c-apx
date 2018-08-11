/*****************************************************************************
* \file      testsuite_apx_clientConnection.c
* \author    Conny Gustafsson
* \date      2018-08-09
* \brief     Description
*
* Copyright (c) 2018 Conny Gustafsson
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "adt_str.h"
#include "ApxNode_TestNode1.h"
#include "apx_client.h"
#include "apx_clientConnection.h"
#include "testsocket_spy.h"
#include "apx_fileManager.h"
#include "rmf.h"
#include "CuTest.h"
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
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_clientConnection_transmitHandlerSetup(CuTest* tc);
static void test_apx_clientConnection_sendGreetingOnConnectEvent(CuTest* tc);
static void test_apx_clientConnection_sendLocalFileInfoWhenReceivingAck(CuTest* tc);
static void testsocket_send_acknowledge(testsocket_t *sock);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_clientConnection(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_clientConnection_transmitHandlerSetup);
   SUITE_ADD_TEST(suite, test_apx_clientConnection_sendGreetingOnConnectEvent);
   SUITE_ADD_TEST(suite, test_apx_clientConnection_sendLocalFileInfoWhenReceivingAck);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_clientConnection_transmitHandlerSetup(CuTest* tc)
{
   apx_client_t *client;
   apx_clientConnection_t *connection;
   apx_fileManager_t *fileManager;
   testsocket_t *sock;
   testsocket_spy_create();
   client = apx_client_new();
   connection = client->connection;
   fileManager = &connection->fileManager;
   sock = testsocket_spy_server();
   apx_clientConnection_connect(connection, sock);
   CuAssertPtrEquals(tc, 0, fileManager->transmitHandler.send);
   testsocket_onConnect(sock);
   CuAssertPtrNotNull(tc, fileManager->transmitHandler.send);
   testsocket_onDisconnect(sock);
   apx_client_delete(client);
   testsocket_spy_destroy();
}

static void test_apx_clientConnection_sendGreetingOnConnectEvent(CuTest* tc)
{
   apx_client_t *client;
   apx_clientConnection_t *connection;
   testsocket_t *sock;
   uint32_t len;
   adt_str_t *str;
   const char *expectedGreeting = "RMFP/1.0\nNumHeader-Format:32\n\n";
   const char *data;
   testsocket_spy_create();
   client = apx_client_new();
   connection = client->connection;
   sock = testsocket_spy_server();
   CuAssertPtrNotNull(tc, sock);
   apx_clientConnection_connect(connection, sock);
   CuAssertIntEquals(tc, 0, testsocket_spy_getServerConnectedCount());
   testsocket_onConnect(sock);
   testsocket_run(sock);
   data = (const char*) testsocket_spy_getReceivedData(&len);
   CuAssertIntEquals(tc, 31, testsocket_spy_getServerBytesReceived());
   CuAssertIntEquals(tc, 30, data[0]);
   str = adt_str_make(&data[1], &data[1]+30);
   CuAssertStrEquals(tc, expectedGreeting, adt_str_cstr(str));
   adt_str_delete(str);
   apx_client_delete(client);
   testsocket_spy_destroy();
}


static void test_apx_clientConnection_sendLocalFileInfoWhenReceivingAck(CuTest* tc)
{
   apx_client_t *client;
   apx_clientConnection_t *connection;
   testsocket_t *sock;
   uint32_t len;
   apx_nodeData_t * nodeData;
   testsocket_spy_create();
   client = apx_client_new();
   connection = client->connection;
   ApxNode_Init_TestNode1();
   nodeData = ApxNode_GetNodeData_TestNode1();
   apx_client_attachLocalNode(client, nodeData);
   sock = testsocket_spy_server();
   CuAssertPtrNotNull(tc, sock);
   apx_clientConnection_connect(connection, sock);
   CuAssertIntEquals(tc, 0, testsocket_spy_getServerConnectedCount());
   testsocket_onConnect(sock);
   testsocket_run(sock);
   CuAssertIntEquals(tc, 31, testsocket_spy_getServerBytesReceived()); //this is the expected acknowledge
   testsocket_spy_clearReceivedData();
   testsocket_send_acknowledge(sock);
   testsocket_run(sock);
   SLEEP(50); //Give some time for fileManager worker thread to process internal messages
   testsocket_run(sock);
   (void) testsocket_spy_getReceivedData(&len);
   CuAssertIntEquals(tc, 134, len); //this is the expected file info data
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
