//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_file.h"
#include "apx_eventFile.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_file_remote(CuTest* tc);
static void test_apx_file_basename(CuTest* tc);
static void test_apx_file_rmf_event_file(CuTest* tc);
static void test_apx_file_addEventListener(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_file(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_file_remote);
   SUITE_ADD_TEST(suite, test_apx_file_basename);
   SUITE_ADD_TEST(suite, test_apx_file_rmf_event_file);
   SUITE_ADD_TEST(suite, test_apx_file_addEventListener);


   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_file_remote(CuTest* tc)
{
   apx_file_t *file1;
   rmf_fileInfo_t info1;
   info1.address = 0x10000;
   strcpy(info1.name,"hello.txt");
   info1.digestType = RMF_DIGEST_TYPE_NONE;
   info1.fileType = RMF_FILE_TYPE_FIXED;
   info1.length = 2542;
   file1 = apx_file_new(APX_UNKNOWN_FILE, &info1);
   CuAssertPtrNotNull(tc, file1);
   CuAssertIntEquals(tc, 1, (int) file1->isRemoteFile);
   CuAssertStrEquals(tc, "hello.txt", file1->fileInfo.name);
   CuAssertIntEquals(tc, 0x10000, (int) file1->fileInfo.address);
   CuAssertIntEquals(tc, 2542, (int) file1->fileInfo.length);
   CuAssertIntEquals(tc, RMF_DIGEST_TYPE_NONE, (int) file1->fileInfo.digestType);
   CuAssertPtrEquals(tc, 0, file1->nodeData);
   CuAssertIntEquals(tc, APX_USER_DATA_FILE, file1->fileType);
   apx_file_delete(file1);
}

static void test_apx_file_basename(CuTest* tc)
{
   apx_file_t *file1;
   rmf_fileInfo_t info1;
   char *str;
   info1.address = 0x10000;
   strcpy(info1.name,"hello.txt");
   info1.digestType = RMF_DIGEST_TYPE_NONE;
   info1.fileType = RMF_FILE_TYPE_FIXED;
   info1.length = 2542;
   file1 = apx_file_new(APX_UNKNOWN_FILE, &info1);
   CuAssertPtrNotNull(tc, file1);
   str = apx_file_basename(file1);
   CuAssertPtrNotNull(tc, str);
   CuAssertStrEquals(tc, "hello", (char*) str);
   free(str);
   apx_file_delete(file1);
}

static void test_apx_file_rmf_event_file(CuTest* tc)
{
   apx_file_t *file1;
   rmf_fileInfo_t info1;
   info1.address = APX_EVENT_LOG_FILE_ADDRESS;
   strcpy(info1.name,APX_EVENT_LOG_FILE_NAME);
   info1.digestType = RMF_DIGEST_TYPE_NONE;
   info1.fileType = RMF_FILE_TYPE_STREAM;
   info1.length = APX_EVENT_LOG_FILE_LEN;
   file1 = apx_file_new(APX_EVENT_FILE, &info1);
   CuAssertPtrNotNull(tc, file1);
   CuAssertUIntEquals(tc, APX_EVENT_LOG_FILE_ADDRESS, file1->fileInfo.address);
   CuAssertUIntEquals(tc, RMF_DIGEST_TYPE_NONE, file1->fileInfo.digestType);
   CuAssertUIntEquals(tc, RMF_FILE_TYPE_STREAM, file1->fileInfo.fileType);
   CuAssertUIntEquals(tc, APX_EVENT_LOG_FILE_LEN, file1->fileInfo.length);
   apx_file_delete(file1);
}

static void test_apx_file_addEventListener(CuTest* tc)
{
   apx_file_t *file1;
   rmf_fileInfo_t info1;
   info1.address = 0x0;
   strcpy(info1.name,"test.out");
   info1.digestType = RMF_DIGEST_TYPE_NONE;
   info1.fileType = RMF_FILE_TYPE_FIXED;
   info1.length = 10;
   file1 = apx_file_new(APX_OUTDATA_FILE, &info1);
   apx_file_delete(file1);
}
