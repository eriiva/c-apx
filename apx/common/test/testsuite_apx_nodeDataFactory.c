/*****************************************************************************
* \file      testsuite_apx_nodeDataFactory.c
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
#include "apx_nodeDataFactory.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataFactory_create(CuTest *tc);
static void test_apx_nodeDataFactory_fromString(CuTest *tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_nodeDataFactory(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_nodeDataFactory_create);
   SUITE_ADD_TEST(suite, test_apx_nodeDataFactory_fromString);


   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataFactory_create(CuTest* tc)
{
   apx_nodeDataFactory_t factory;

   apx_nodeDataFactory_create(&factory);
   CuAssertUIntEquals(tc, APX_NODE_DATA_FACTORY_NO_ERROR, factory.lastError);

   apx_nodeDataFactory_destroy(&factory);
}

static void test_apx_nodeDataFactory_fromString(CuTest *tc)
{
   apx_nodeDataFactory_t factory;
   apx_nodeData_t *nodeData;
   const char *test_definition=
         "APX/1.2\n"
         "N\"TestNode\"\n"
         "P\"OutPort1\"C(0,1):=0\n";
   apx_nodeDataFactory_create(&factory);

   nodeData = apx_nodeDataFactory_fromString(&factory, (uint8_t*) test_definition, (uint32_t) strlen(test_definition));

   CuAssertPtrNotNull(tc, nodeData);
   apx_nodeData_delete(nodeData);
   apx_nodeDataFactory_destroy(&factory);
}
