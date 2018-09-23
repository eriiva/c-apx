/*****************************************************************************
* \file      testsuite_apx_nodeDataManager.c
* \author    Conny Gustafsson
* \date      2018-09-03
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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "CuTest.h"
#include "apx_nodeDataManager.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataManager_create(CuTest *tc);
static void test_apx_nodeDataManager_fromValidString(CuTest *tc);
static void test_apx_nodeDataManager_fromInvalidString1(CuTest *tc);
static void test_apx_nodeDataManager_fromInvalidString2(CuTest *tc);
static void test_apx_nodeDataManager_createWithoutInitialName(CuTest *tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_nodeDataManager(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_nodeDataManager_create);
   SUITE_ADD_TEST(suite, test_apx_nodeDataManager_fromValidString);
   SUITE_ADD_TEST(suite, test_apx_nodeDataManager_fromInvalidString1);
   SUITE_ADD_TEST(suite, test_apx_nodeDataManager_fromInvalidString2);
   SUITE_ADD_TEST(suite, test_apx_nodeDataManager_createWithoutInitialName);


   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataManager_create(CuTest* tc)
{
   apx_nodeDataManager_t factory;

   apx_nodeDataManager_create(&factory);

   apx_nodeDataManager_destroy(&factory);
}

static void test_apx_nodeDataManager_fromValidString(CuTest *tc)
{
   apx_nodeDataManager_t manager;
   apx_nodeData_t *nodeData;
   apx_error_t lastError;
   const char *test_definition=
         "APX/1.2\n"
         "N\"TestNode\"\n"
         "P\"OutPort1\"C(0,1):=0\n";
   apx_nodeDataManager_create(&manager);
   nodeData = apx_nodeDataManager_newNodeData(&manager, "TestNode");

   lastError = apx_nodeDataManager_parseDefinition(&manager, nodeData, (uint8_t*) test_definition, (uint32_t) strlen(test_definition) );
   CuAssertIntEquals(tc, APX_NO_ERROR, lastError);

   CuAssertPtrNotNull(tc, nodeData);
   apx_nodeData_delete(nodeData);
   apx_nodeDataManager_destroy(&manager);
}

static void test_apx_nodeDataManager_fromInvalidString1(CuTest *tc)
{
   apx_nodeDataManager_t manager;
   apx_nodeData_t *nodeData;
   apx_error_t lastError;
   const char *test_definition=
         "APX/1.2\n"
         "N\"TestNode\"" //missing the new-line character here
         "P\"OutPort1\"C(0,1):=0\n";
   apx_nodeDataManager_create(&manager);

   nodeData = apx_nodeDataManager_newNodeData(&manager, "TestNode");
   lastError = apx_nodeDataManager_parseDefinition(&manager, nodeData, (uint8_t*) test_definition, (uint32_t) strlen(test_definition) );
   CuAssertIntEquals(tc, APX_PARSE_ERROR, lastError);

   CuAssertIntEquals(tc, 2, apx_nodeDataManager_getErrorLine(&manager));
   apx_nodeData_delete(nodeData);
   apx_nodeDataManager_destroy(&manager);

}

static void test_apx_nodeDataManager_fromInvalidString2(CuTest *tc)
{
   apx_nodeDataManager_t manager;
   apx_nodeData_t *nodeData;
   apx_error_t lastError;
   const char *test_definition=
         "APX/1.2\n"
         "N\"TestNode\"" //missing the new-line character here
         "R\"OutPort1\"a[10]:\n";
   apx_nodeDataManager_create(&manager);

   nodeData = apx_nodeDataManager_newNodeData(&manager, "TestNode");
   lastError = apx_nodeDataManager_parseDefinition(&manager, nodeData, (uint8_t*) test_definition, (uint32_t) strlen(test_definition) );
   CuAssertIntEquals(tc, APX_PARSE_ERROR, lastError);

   CuAssertIntEquals(tc, 2, apx_nodeDataManager_getErrorLine(&manager));
   apx_nodeData_delete(nodeData);
   apx_nodeDataManager_destroy(&manager);

}

static void test_apx_nodeDataManager_createWithoutInitialName(CuTest *tc)
{
   apx_nodeDataManager_t manager;
   apx_nodeData_t *nodeData;
   apx_error_t lastError;
   const char *test_definition=
         "APX/1.2\n"
         "N\"TestNode\"\n"
         "P\"OutPort1\"C(0,1):=0\n";
   apx_nodeDataManager_create(&manager);
   nodeData = apx_nodeDataManager_newNodeData(&manager, NULL);

   CuAssertPtrNotNull(tc, nodeData);
   CuAssertPtrEquals(tc, NULL, (char*) apx_nodeData_getName(nodeData));
   lastError = apx_nodeDataManager_parseDefinition(&manager, nodeData, (uint8_t*) test_definition, (uint32_t) strlen(test_definition) );
   CuAssertIntEquals(tc, APX_NO_ERROR, lastError);

   CuAssertStrEquals(tc, "TestNode", apx_nodeData_getName(nodeData));
   apx_nodeData_delete(nodeData);
   apx_nodeDataManager_destroy(&manager);
}
