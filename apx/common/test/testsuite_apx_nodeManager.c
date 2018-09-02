/*****************************************************************************
* \file      testsuite_apx_nodeManager.c
* \author    Conny Gustafsson
* \date      2018-08-10
* \brief     Unit tests for apx_nodeManager
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
#include "ApxNode_TestNode1.h"
#include "apx_fileManager.h"
#include "apx_nodeManager.h"
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define DEFAULT_CONNECTION_ID 0
//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeManager_create(CuTest* tc);
static void test_apx_nodeManager_attachLocalNode(CuTest *tc);
//static void test_apx_nodeManager_attachFileManagerWithOneLocalNodePresent(CuTest *tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_nodeManager(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_nodeManager_create);
   SUITE_ADD_TEST(suite, test_apx_nodeManager_attachLocalNode);
   //SUITE_ADD_TEST(suite, test_apx_nodeManager_attachFileManagerWithOneLocalNodePresent);

   return suite;
}
//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeManager_create(CuTest* tc)
{
   apx_nodeManager_t nodeManager;
   apx_nodeManager_create(&nodeManager);
   CuAssertPtrEquals(tc, 0, nodeManager.router);
   apx_nodeManager_destroy(&nodeManager);
}

static void test_apx_nodeManager_attachLocalNode(CuTest *tc)
{
   apx_nodeData_t *nodeData1;
   apx_nodeData_t *nodeData2;
   apx_nodeManager_t nodeManager;
   apx_nodeManager_create(&nodeManager);
   ApxNode_Init_TestNode1();
   nodeData1 = ApxNode_GetNodeData_TestNode1();
   apx_nodeManager_attachLocalNode(&nodeManager, nodeData1);
   nodeData2 = apx_nodeManager_findNodeData(&nodeManager, nodeData1->name);
   CuAssertPtrEquals(tc, nodeData1, nodeData2);
   apx_nodeManager_destroy(&nodeManager);
}

/*
static void test_apx_nodeManager_attachFileManagerWithOneLocalNodePresent(CuTest *tc)
{
   apx_nodeData_t *nodeData;

   apx_nodeManager_t nodeManager;
   apx_fileManager_t fileManager;
   apx_nodeManager_create(&nodeManager);
   ApxNode_Init_TestNode1();
   nodeData = ApxNode_GetNodeData_TestNode1();
   apx_nodeManager_attachLocalNode(&nodeManager, nodeData);
   apx_fileManager_create(&fileManager, APX_FILEMANAGER_CLIENT_MODE, DEFAULT_CONNECTION_ID);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumLocalFiles(&fileManager));
   apx_nodeManager_attachFileManager(&nodeManager, &fileManager);
   CuAssertIntEquals(tc, 2, apx_fileManager_getNumLocalFiles(&fileManager));
   apx_nodeManager_destroy(&nodeManager);
   apx_fileManager_destroy(&fileManager);
}
*/
