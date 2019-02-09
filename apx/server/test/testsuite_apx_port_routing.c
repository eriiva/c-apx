/*****************************************************************************
* \file      testsuite_apx_connection_routing.c
* \author    Conny Gustafsson
* \date      2018-12-09
* \brief     Unit tests for testing that ports are connected properly between APX nodes
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
#include <stddef.h>
#include <stdio.h>
#include "CuTest.h"
#include "apx_server.h"
#include "apx_test_nodes.h"
#include "apx_serverTestConnection.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_SERVER_RUN(srv, sock) testsocket_run(sock); apx_server_run(srv); SLEEP(50); testsocket_run(sock); apx_server_run(srv)

#define APX_PORT_DATA_START_ADDRESS 0x0
#define APX_DEF_START_ADDRESS 0x4000000

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_port_routing_connect_nodes(CuTest* tc);
static void test_apx_port_routing_close_connection(CuTest* tc);

static void attachTestNode1(apx_server_t *pServer, apx_serverTestConnection_t *pConnection);
static void attachTestNode5(apx_server_t *pServer, apx_serverTestConnection_t *pConnection);
static void closeConnection(apx_server_t *pServer, apx_serverTestConnection_t *pConnection);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////
uint8_t m_outPortDataNode1[APX_TESTNODE1_OUT_DATA_LEN];
uint8_t m_outPortDataNode5[APX_TESTNODE5_OUT_DATA_LEN];
//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_port_routing(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_port_routing_connect_nodes);
   SUITE_ADD_TEST(suite, test_apx_port_routing_close_connection);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_port_routing_connect_nodes(CuTest* tc)
{
   apx_server_t server, *pServer;
   apx_serverTestConnection_t *connection1;
   apx_serverTestConnection_t *connection2;

   apx_routingTable_t *routingTable;

   pServer = &server;

   apx_server_create(pServer);
   routingTable = apx_server_getRoutingTable(pServer);
   connection1 = apx_serverTestConnection_new(pServer);
   connection2 = apx_serverTestConnection_new(pServer);
   apx_server_acceptConnection(pServer, (apx_serverConnectionBase_t*) connection1);
   apx_server_acceptConnection(pServer, (apx_serverConnectionBase_t*) connection2);

   CuAssertUIntEquals(tc, 0, apx_routingTable_length(routingTable));
   attachTestNode1(pServer, connection1);
   CuAssertUIntEquals(tc, 4, apx_routingTable_length(routingTable));

   attachTestNode5(pServer, connection2);
   CuAssertUIntEquals(tc, 4, apx_routingTable_length(routingTable));

   apx_server_destroy(pServer);
}

static void test_apx_port_routing_close_connection(CuTest* tc)
{
   apx_server_t server, *pServer;
   apx_serverTestConnection_t *connection1;
   apx_serverTestConnection_t *connection2;
   apx_nodeData_t *nodeData1;
   apx_nodeData_t *nodeData5;

   apx_routingTable_t *routingTable;

   pServer = &server;

   apx_server_create(pServer);
   routingTable = apx_server_getRoutingTable(pServer);
   connection1 = apx_serverTestConnection_new(pServer);
   connection2 = apx_serverTestConnection_new(pServer);
   apx_server_acceptConnection(pServer, (apx_serverConnectionBase_t*) connection1);
   apx_server_acceptConnection(pServer, (apx_serverConnectionBase_t*) connection2);

   CuAssertUIntEquals(tc, 0, apx_routingTable_length(routingTable));
   attachTestNode1(pServer, connection1);
   CuAssertUIntEquals(tc, 4, apx_routingTable_length(routingTable));
   nodeData1 = apx_nodeDataManager_find(&connection1->base.base.nodeDataManager, "TestNode1");
   CuAssertPtrNotNull(tc, nodeData1);
   CuAssertUIntEquals(tc, 0, apx_nodeData_getPortConnectionsTotal(nodeData1));

   attachTestNode5(pServer, connection2);
   CuAssertUIntEquals(tc, 4, apx_routingTable_length(routingTable));
   nodeData5 = apx_nodeDataManager_find(&connection2->base.base.nodeDataManager, "TestNode5");
   CuAssertPtrNotNull(tc, nodeData5);

   CuAssertUIntEquals(tc, 3, apx_nodeData_getPortConnectionsTotal(nodeData1));
   CuAssertUIntEquals(tc, 3, apx_nodeData_getPortConnectionsTotal(nodeData5));

   closeConnection(pServer, connection1); //removes TestNode1 from routing table
   CuAssertUIntEquals(tc, 3, apx_routingTable_length(routingTable));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getPortConnectionsTotal(nodeData5));
   closeConnection(pServer, connection2);

   apx_server_destroy(pServer);
}

static void attachTestNode1(apx_server_t *pServer, apx_serverTestConnection_t *pConnection)
{
   rmf_fileInfo_t fileInfo, *pFileinfo;
   uint32_t testNode1Len;

   pFileinfo = &fileInfo;
   memset(m_outPortDataNode1, 0xFF, APX_TESTNODE1_OUT_DATA_LEN);
   testNode1Len = (uint32_t) strlen(g_apx_test_node1);
   rmf_fileInfo_create(pFileinfo, "TestNode1.apx", APX_DEF_START_ADDRESS, testNode1Len, RMF_FILE_TYPE_FIXED);
   apx_serverTestConnection_createRemoteFile(pConnection, pFileinfo);
   rmf_fileInfo_create(pFileinfo, "TestNode1.out", APX_PORT_DATA_START_ADDRESS, APX_TESTNODE1_OUT_DATA_LEN, RMF_FILE_TYPE_FIXED);
   apx_serverTestConnection_createRemoteFile(pConnection, pFileinfo);
   apx_server_run(pServer);
   apx_serverTestConnection_writeRemoteData(pConnection, APX_DEF_START_ADDRESS, (const uint8_t*) g_apx_test_node1, testNode1Len, false);
   apx_server_run(pServer);
   apx_serverTestConnection_writeRemoteData(pConnection, APX_PORT_DATA_START_ADDRESS, m_outPortDataNode1, APX_TESTNODE1_OUT_DATA_LEN, false);
   apx_serverTestConnection_openRemoteFile(pConnection, APX_PORT_DATA_START_ADDRESS);
   apx_server_run(pServer);
}

static void attachTestNode5(apx_server_t *pServer, apx_serverTestConnection_t *pConnection)
{
   rmf_fileInfo_t fileInfo, *pFileinfo;
   uint32_t testNode5Len;

   pFileinfo = &fileInfo;
   testNode5Len = (uint32_t) strlen(g_apx_test_node5);
   memset(m_outPortDataNode5, 0xFF, APX_TESTNODE5_OUT_DATA_LEN);
   rmf_fileInfo_create(pFileinfo, "TestNode5.apx", APX_DEF_START_ADDRESS, testNode5Len, RMF_FILE_TYPE_FIXED);
   apx_serverTestConnection_createRemoteFile(pConnection, pFileinfo);
   rmf_fileInfo_create(pFileinfo, "TestNode5.out", APX_PORT_DATA_START_ADDRESS, APX_TESTNODE5_OUT_DATA_LEN, RMF_FILE_TYPE_FIXED);
   apx_serverTestConnection_createRemoteFile(pConnection, pFileinfo);
   apx_server_run(pServer);
   apx_serverTestConnection_writeRemoteData(pConnection, APX_DEF_START_ADDRESS, (const uint8_t*) g_apx_test_node5, testNode5Len, false);
   apx_server_run(pServer);
   apx_serverTestConnection_writeRemoteData(pConnection, APX_PORT_DATA_START_ADDRESS, m_outPortDataNode5, APX_TESTNODE5_OUT_DATA_LEN, false);
   apx_serverTestConnection_openRemoteFile(pConnection, APX_PORT_DATA_START_ADDRESS);
   apx_server_run(pServer);
}

static void closeConnection(apx_server_t *pServer, apx_serverTestConnection_t *pConnection)
{
   apx_server_closeConnection(pServer, (apx_serverConnectionBase_t*) pConnection);
   apx_server_run(pServer);

}


