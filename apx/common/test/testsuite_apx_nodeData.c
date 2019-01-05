//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "CuTest.h"
#include "apx_nodeData.h"
#include "apx_parser.h"
#include "apx_test_nodes.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeData_newEmpty(CuTest* tc);
static void test_apx_nodeData_outPortConnectionCount(CuTest* tc);
static void test_apx_nodeData_inPortConnectionCount(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_nodeData(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_nodeData_newEmpty);
   SUITE_ADD_TEST(suite, test_apx_nodeData_outPortConnectionCount);
   SUITE_ADD_TEST(suite, test_apx_nodeData_inPortConnectionCount);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_nodeData_newEmpty(CuTest* tc)
{
   apx_nodeData_t *nodeData;


   nodeData = apx_nodeData_new(100u);
   CuAssertPtrNotNull(tc, nodeData);
   apx_nodeData_delete(nodeData);

}

static void test_apx_nodeData_outPortConnectionCount(CuTest* tc)
{
   apx_parser_t parser;
   apx_parser_create(&parser);
   apx_nodeData_t *nodeData;

   nodeData = apx_nodeData_makeFromString(&parser, g_apx_test_node1); //This APX node has 3 output ports and 1 input port
   CuAssertPtrNotNull(tc, nodeData);

   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 2));

   apx_nodeData_incOutPortConnectionCount(nodeData, 0);
   apx_nodeData_incOutPortConnectionCount(nodeData, 0);
   apx_nodeData_incOutPortConnectionCount(nodeData, 0);
   apx_nodeData_incOutPortConnectionCount(nodeData, 1);
   apx_nodeData_incOutPortConnectionCount(nodeData, 1);
   apx_nodeData_incOutPortConnectionCount(nodeData, 2);
   CuAssertUIntEquals(tc, 3, apx_nodeData_getOutPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 2, apx_nodeData_getOutPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 1, apx_nodeData_getOutPortConnectionCount(nodeData, 2));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 3));
   apx_nodeData_decOutPortConnectionCount(nodeData, 0);
   apx_nodeData_decOutPortConnectionCount(nodeData, 1);
   apx_nodeData_decOutPortConnectionCount(nodeData, 2);
   CuAssertUIntEquals(tc, 2, apx_nodeData_getOutPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 1, apx_nodeData_getOutPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 2));
   //decreasing a count that is already at zero shall stay at zero
   CuAssertUIntEquals(tc, 0, apx_nodeData_getOutPortConnectionCount(nodeData, 2));
   apx_nodeData_decOutPortConnectionCount(nodeData, 2);
   apx_parser_destroy(&parser);
   apx_nodeData_delete(nodeData);
}


static void test_apx_nodeData_inPortConnectionCount(CuTest* tc)
{
   apx_parser_t parser;
   apx_parser_create(&parser);
   apx_nodeData_t *nodeData;

   nodeData = apx_nodeData_makeFromString(&parser, g_apx_test_node4); //This APX node has 0 output ports and 3 input ports
   CuAssertPtrNotNull(tc, nodeData);

   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 2));

   apx_nodeData_incInPortConnectionCount(nodeData, 0);
   apx_nodeData_incInPortConnectionCount(nodeData, 0);
   apx_nodeData_incInPortConnectionCount(nodeData, 0);
   apx_nodeData_incInPortConnectionCount(nodeData, 1);
   apx_nodeData_incInPortConnectionCount(nodeData, 1);
   apx_nodeData_incInPortConnectionCount(nodeData, 2);
   CuAssertUIntEquals(tc, 3, apx_nodeData_getInPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 2, apx_nodeData_getInPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 1, apx_nodeData_getInPortConnectionCount(nodeData, 2));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 3));
   apx_nodeData_decInPortConnectionCount(nodeData, 0);
   apx_nodeData_decInPortConnectionCount(nodeData, 1);
   apx_nodeData_decInPortConnectionCount(nodeData, 2);
   CuAssertUIntEquals(tc, 2, apx_nodeData_getInPortConnectionCount(nodeData, 0));
   CuAssertUIntEquals(tc, 1, apx_nodeData_getInPortConnectionCount(nodeData, 1));
   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 2));
   //decreasing a count that is already at zero shall stay at zero
   CuAssertUIntEquals(tc, 0, apx_nodeData_getInPortConnectionCount(nodeData, 2));
   apx_nodeData_decInPortConnectionCount(nodeData, 2);
   apx_parser_destroy(&parser);
   apx_nodeData_delete(nodeData);
}
