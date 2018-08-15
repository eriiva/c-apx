//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "ApxNode_TestNode1.h"
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
static void test_apx_fileManager_createInServerMode(CuTest* tc);
static void test_apx_fileManager_createInClientMode(CuTest* tc);
static void test_apx_fileManager_attachLocalFiles(CuTest* tc);
static void test_apx_fileManager_registerListener(CuTest* tc);
static void test_apx_fileManager_createRemoteFile(CuTest* tc);

static void eventListenerSpy_init(void);
static void eventListenerSpy_fileCreate(void *arg, apx_fileManager_t *fileManager, apx_file_t *file);

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

   SUITE_ADD_TEST(suite, test_apx_fileManager_createInServerMode);
   SUITE_ADD_TEST(suite, test_apx_fileManager_createInClientMode);
   SUITE_ADD_TEST(suite, test_apx_fileManager_attachLocalFiles);
   SUITE_ADD_TEST(suite, test_apx_fileManager_registerListener);
   SUITE_ADD_TEST(suite, test_apx_fileManager_createRemoteFile);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_apx_fileManager_createInServerMode(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumEventListeners(&manager));
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_createInClientMode(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_CLIENT_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_attachLocalFiles(CuTest* tc)
{
   apx_fileManagerEventListener_t listener;
   apx_nodeData_t *nodeData;
   apx_file_t *definitionFile;
   apx_file_t *outDataFile;
   memset(&listener, 0, sizeof(listener));
   eventListenerSpy_init();
   listener.fileCreate = eventListenerSpy_fileCreate;
   apx_fileManager_t manager;
   apx_fileManager_create(&manager, APX_FILEMANAGER_CLIENT_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_registerEventListener(&manager, &listener);
   ApxNode_Init_TestNode1();
   nodeData = ApxNode_GetNodeData_TestNode1();
   definitionFile = apx_file_newLocalDefinitionFile(nodeData);
   outDataFile = apx_file_newLocalOutPortDataFile(nodeData);
   CuAssertPtrNotNull(tc, definitionFile);
   CuAssertPtrNotNull(tc, outDataFile);
   CuAssertIntEquals(tc, 0, m_numfileCreateCalls);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumLocalFiles(&manager));
   apx_fileManager_attachLocalFile(&manager, definitionFile);
   apx_fileManager_attachLocalFile(&manager, outDataFile);
   CuAssertIntEquals(tc, 2, m_numfileCreateCalls);
   CuAssertIntEquals(tc, 2, apx_fileManager_getNumLocalFiles(&manager));
   CuAssertTrue(tc, !m_lastFile->isRemoteFile);
   CuAssertTrue(tc, !m_lastFile->isOpen);
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_registerListener(CuTest* tc)
{
   apx_fileManager_t manager;
   apx_fileManagerEventListener_t eventListener;
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

static void test_apx_fileManager_createRemoteFile(CuTest* tc)
{
   apx_fileManagerEventListener_t listener;
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
   CuAssertTrue(tc, m_lastFile->isRemoteFile);
   CuAssertTrue(tc, !m_lastFile->isOpen);
   apx_fileManager_destroy(&manager);
}

static void eventListenerSpy_init(void)
{
   m_numfileCreateCalls = 0;
   m_lastFileManager = 0;
   m_lastFile = 0;
}

static void eventListenerSpy_fileCreate(void *arg, apx_fileManager_t *fileManager, apx_file_t *file)
{
   m_numfileCreateCalls++;
   m_lastFileManager = fileManager;
   m_lastFile = file;
}
