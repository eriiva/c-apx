/*****************************************************************************
* \file      testsuite_apx_client.c
* \author    Conny Gustafsson
* \date      2018-08-08
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
#include "ApxNode_TestNode1.h"
#include "apx_client.h"
#include "testsocket.h"
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_client_create(CuTest* tc);
static void test_apx_client_connect(CuTest* tc);
static void test_apx_client_attachLocalNode(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_client(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_client_create);
   SUITE_ADD_TEST(suite, test_apx_client_connect);
   SUITE_ADD_TEST(suite, test_apx_client_attachLocalNode);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_client_create(CuTest* tc)
{
   apx_client_t *client = apx_client_new();
   CuAssertPtrNotNull(tc, client);
   apx_client_delete(client);
}

static void test_apx_client_connect(CuTest* tc)
{
   testsocket_t *sock;
   apx_client_t *client = apx_client_new();
   sock = testsocket_new();
   CuAssertPtrNotNull(tc, client);
   apx_client_delete(client);
   testsocket_delete(sock);
}

static void test_apx_client_attachLocalNode(CuTest* tc)
{
   apx_nodeData_t *node;
   apx_client_t *client = apx_client_new();
   ApxNode_Init_TestNode1();
   node = ApxNode_GetNodeData_TestNode1();
   CuAssertPtrNotNull(tc, node);
   apx_client_attachLocalNode(client, node);
   apx_client_delete(client);
}
