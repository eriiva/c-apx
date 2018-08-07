//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <string.h>
#include "CuTest.h"
#include <apx_fileManagerRemote.h>
#include "rmf.h"
#include "apx_file.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct fileManagerRemoteSpy_tag
{
   int32_t fileCreatedByRemoteCalls;
   void *arg;
   apx_file_t file;
}fileManagerRemoteSpy_t;
//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManagerRemote_create(CuTest* tc);
static void test_apx_fileManagerRemote_processFileInfo(CuTest* tc);
static void fileManagerRemoteSpy_init(void);
static void fileCreatedByRemoteSpy(void *arg, const struct apx_file_tag *pFile);

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
static fileManagerRemoteSpy_t m_spy;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_fileManagerRemote(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_fileManagerRemote_create);
   SUITE_ADD_TEST(suite, test_apx_fileManagerRemote_processFileInfo);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManagerRemote_create(CuTest* tc)
{
   apx_fileManagerRemote_t remote;
   apx_fileManagerShared_t shared;
   CuAssertIntEquals(tc, 0, apx_fileManagerShared_create(&shared, 0));
   apx_fileManagerRemote_create(&remote, &shared);
   CuAssertIntEquals(tc, 0, apx_fileMap_length(&remote.remoteFileMap));
   apx_fileManagerRemote_destroy(&remote);
   apx_fileManagerShared_destroy(&shared);
}

static void test_apx_fileManagerRemote_processFileInfo(CuTest* tc)
{
   apx_fileManagerRemote_t remote;
   apx_fileManagerShared_t shared;
   uint8_t buffer[100];
   rmf_fileInfo_t info;
   int32_t msgLen;
   CuAssertIntEquals(tc, 0, apx_fileManagerShared_create(&shared, 0));
   apx_fileManagerRemote_create(&remote, &shared);
   shared.fileCreatedByRemote = fileCreatedByRemoteSpy;
   fileManagerRemoteSpy_init();
   rmf_fileInfo_create(&info, "test.apx", 0x10000, 100, RMF_FILE_TYPE_FIXED);
   msgLen = rmf_packHeader(&buffer[0], sizeof(buffer), RMF_CMD_START_ADDR, false);
   msgLen += rmf_serialize_cmdFileInfo(&buffer[msgLen], sizeof(buffer)-msgLen, &info);
   CuAssertIntEquals(tc, msgLen, apx_fileManagerRemote_parseMessage(&remote, &buffer[0], msgLen));
   CuAssertIntEquals(tc, 1, m_spy.fileCreatedByRemoteCalls);
   apx_fileManagerRemote_destroy(&remote);
   apx_fileManagerShared_destroy(&shared);
}

static void fileManagerRemoteSpy_init(void)
{
   memset(&m_spy, 0, sizeof(m_spy));
}

static void fileCreatedByRemoteSpy(void *arg, const struct apx_file_tag *pFile)
{
   assert(pFile != 0);
   m_spy.fileCreatedByRemoteCalls++;
   m_spy.arg = arg;
   m_spy.file = *pFile;
}
