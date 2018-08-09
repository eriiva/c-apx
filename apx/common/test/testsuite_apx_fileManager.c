//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_file.h"
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
static void test_apx_fileManager_server_create(CuTest* tc);
static void test_apx_fileManager_client_create(CuTest* tc);
static void test_apx_fileManager_createLocalFiles(CuTest* tc);
static void test_apx_fileManager_attachLocalFiles(CuTest* tc);
static void test_apx_fileManager_registerListener(CuTest* tc);
static void test_apx_fileManager_eventListener_createFile(CuTest* tc);
static void test_apx_fileManager_eventListener_createLocalFiles(CuTest* tc);
static void test_apx_fileManager_eventListener_sendLocalFiles(CuTest* tc);

static void eventListenerSpy_init(void);
static void eventListenerSpy_fileCreate(void *arg, apx_fileManager_t *fileManager, const apx_file_t *file);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
int32_t m_numfileCreateCalls;
apx_fileManager_t *m_lastFileManager;
const apx_file_t *m_lastFile;


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testSuite_apx_fileManager(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_fileManager_server_create);
   SUITE_ADD_TEST(suite, test_apx_fileManager_client_create);
   SUITE_ADD_TEST(suite, test_apx_fileManager_createLocalFiles);
   SUITE_ADD_TEST(suite, test_apx_fileManager_attachLocalFiles);
   SUITE_ADD_TEST(suite, test_apx_fileManager_registerListener);
   SUITE_ADD_TEST(suite, test_apx_fileManager_eventListener_createFile);
   SUITE_ADD_TEST(suite, test_apx_fileManager_eventListener_sendLocalFiles);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManager_server_create(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumEventListeners(&manager));
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_client_create(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_CLIENT_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_createLocalFiles(CuTest* tc)
{

}

static void test_apx_fileManager_attachLocalFiles(CuTest* tc)
{

}

static void test_apx_fileManager_registerListener(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_eventListener_t eventListener;
   void *handle;
   memset(&eventListener, 0, sizeof(eventListener));
   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumEventListeners(&manager));
   handle = apx_fileManager_registerEventListener(&manager, &eventListener);
   CuAssertIntEquals(tc, 1, apx_fileManager_getNumEventListeners(&manager));
   apx_fileManager_unregisterEventListener(&manager, handle);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumEventListeners(&manager));

   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_eventListener_createFile(CuTest* tc)
{
   apx_eventListener_t listener;
   uint8_t buffer[100];
   rmf_fileInfo_t info;
   int32_t msgLen;

   memset(&listener, 0, sizeof(listener));
   eventListenerSpy_init();
   listener.fileCreate = eventListenerSpy_fileCreate;
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_registerEventListener(&manager, &listener);
   rmf_fileInfo_create(&info, "test.apx", 0x10000, 100, RMF_FILE_TYPE_FIXED);
   msgLen = rmf_packHeader(&buffer[0], sizeof(buffer), RMF_CMD_START_ADDR, false);
   msgLen += rmf_serialize_cmdFileInfo(&buffer[msgLen], sizeof(buffer)-msgLen, &info);
   CuAssertIntEquals(tc, msgLen, apx_fileManager_parseMessage(&manager, &buffer[0], msgLen));
   CuAssertIntEquals(tc, 1, m_numfileCreateCalls);
   apx_fileManager_destroy(&manager);
}

static void eventListenerSpy_init(void)
{
   m_numfileCreateCalls = 0;
   m_lastFileManager = 0;
   m_lastFile = 0;
}

static void eventListenerSpy_fileCreate(void *arg, apx_fileManager_t *fileManager, const apx_file_t *file)
{
   m_numfileCreateCalls++;
   m_lastFileManager = fileManager;
   m_lastFile = file;
}

static void test_apx_fileManager_eventListener_sendLocalFiles(CuTest* tc)
{

}
