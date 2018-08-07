//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_fileManagerShared.h"
#include "apx_eventListener.h"
#include "soa.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define CONNECTION_ID_DEFAULT 0

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManagerShared_create(CuTest* tc);
static void test_apx_fileManagerShared_alloc(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_fileManagerShared(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_fileManagerShared_create);
   SUITE_ADD_TEST(suite, test_apx_fileManagerShared_alloc);


   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManagerShared_create(CuTest* tc)
{
   apx_fileManagerShared_t data;
   CuAssertIntEquals(tc, 0, apx_fileManagerShared_create(&data, CONNECTION_ID_DEFAULT));
   CuAssertUIntEquals(tc, 0, data.fmid);

   CuAssertPtrEquals(tc, 0, data.fileOpenRequestedByRemote);
   CuAssertPtrEquals(tc, 0, data.fileCreatedByRemote);
   CuAssertTrue(tc, apx_allocator_isRunning(&data.allocator));
   apx_fileManagerShared_destroy(&data);
}

static void test_apx_fileManagerShared_alloc(CuTest* tc)
{
   apx_fileManagerShared_t data;
   uint8_t *ptr;
   size_t size;
   int i;
   apx_fileManagerShared_create(&data, CONNECTION_ID_DEFAULT);
   //allocate small objects
   for(i=1;i<SMALL_OBJECT_MAX_SIZE;i++)
   {
      char msg[20];
      size = i;
      ptr = apx_fileManagerShared_alloc(&data, size);
      sprintf(msg, "size=%d", i);
      CuAssertPtrNotNullMsg(tc, msg, ptr);
      apx_fileManagerShared_free(&data, ptr, size);
   }
   //allocate some large objects
   size = 100;
   ptr = apx_fileManagerShared_alloc(&data, size);
   CuAssertPtrNotNull(tc, ptr);
   apx_fileManagerShared_free(&data, ptr, size);
   size = 1000;
   ptr = apx_fileManagerShared_alloc(&data, size);
   CuAssertPtrNotNull(tc, ptr);
   apx_fileManagerShared_free(&data, ptr, size);
   size = 10000;
   ptr = apx_fileManagerShared_alloc(&data, size);
   CuAssertPtrNotNull(tc, ptr);
   apx_fileManagerShared_free(&data, ptr, size);

   apx_fileManagerShared_destroy(&data);
}
