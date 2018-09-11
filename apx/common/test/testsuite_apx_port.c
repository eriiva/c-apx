//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_port.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_port_create(CuTest* tc);
static void test_apx_port_create_invalidRecordDsg(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testsuite_apx_port(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_port_create);
   SUITE_ADD_TEST(suite, test_apx_port_create_invalidRecordDsg);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_port_create(CuTest* tc)
{
   apx_port_t port;
   //const char *psg;
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_port_create(&port, APX_REQUIRE_PORT, NULL, "C", NULL, 0));
   CuAssertPtrEquals(tc,NULL,port.portAttributes);
   CuAssertPtrEquals(tc,NULL,port.name);
   CuAssertPtrEquals(tc,NULL,port.portSignature);
   apx_port_destroy(&port);

   CuAssertIntEquals(tc, APX_NO_ERROR, apx_port_create(&port,APX_REQUIRE_PORT,"SootLevel","T[95]",NULL, 0));
   //apx_port_setDerivedDataSignature(&port,"C");
   //psg = apx_port_getPortSignature(&port);

   //CuAssertStrEquals(tc,"\"DPFSootLevel\"C",psg);
   apx_port_destroy(&port);

}

static void test_apx_port_create_invalidRecordDsg(CuTest* tc)
{
   apx_port_t port;
   CuAssertIntEquals(tc, APX_UNMATCHED_BRACE_ERROR, apx_port_create(&port, APX_REQUIRE_PORT, "Hello", "{\"UserId\"S", "=0", 0));
}





