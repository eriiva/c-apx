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
#include "apx_file2.h"
#include "apx_transmitHandlerSpy.h"
#include "apx_fileManagerEventListenerSpy.h"
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
static void test_apx_fileManager_openRemoteFile_sendMessage(CuTest* tc);
static void test_apx_fileManager_openRemoteFile_setOpenFlag(CuTest* tc);
static void test_apx_fileManager_openRemoteFile_processRequest_fixedFileNoReadHandler(CuTest* tc);
static void test_apx_fileManager_openRemoteFile_processRequest_fixedFileFromNodeData(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


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
   SUITE_ADD_TEST(suite, test_apx_fileManager_openRemoteFile_sendMessage);
   SUITE_ADD_TEST(suite, test_apx_fileManager_openRemoteFile_setOpenFlag);
   SUITE_ADD_TEST(suite, test_apx_fileManager_openRemoteFile_processRequest_fixedFileNoReadHandler);
   SUITE_ADD_TEST(suite, test_apx_fileManager_openRemoteFile_processRequest_fixedFileFromNodeData);

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
   apx_file2_t *definitionFile;
   apx_file2_t *outDataFile;
   apx_fileManagerEventListenerSpy_t spy;
   apx_fileManager_t manager;

   apx_fileManagerEventListenerSpy_create(&spy);
   memset(&listener, 0, sizeof(listener));
   listener.arg = &spy;
   listener.fileCreate = apx_fileManagerEventListenerSpy_fileCreate;
   apx_fileManager_create(&manager, APX_FILEMANAGER_CLIENT_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_registerEventListener(&manager, &listener);
   ApxNode_Init_TestNode1();
   nodeData = ApxNode_GetNodeData_TestNode1();
   definitionFile = apx_nodeData_newLocalDefinitionFile(nodeData);
   outDataFile = apx_nodeData_newLocalOutPortDataFile(nodeData);
   CuAssertPtrNotNull(tc, definitionFile);
   CuAssertPtrNotNull(tc, outDataFile);
   CuAssertIntEquals(tc, 0, spy.numfileCreateCalls);
   CuAssertIntEquals(tc, 0, apx_fileManager_getNumLocalFiles(&manager));
   apx_fileManager_attachLocalFile(&manager, definitionFile);
   apx_fileManager_attachLocalFile(&manager, outDataFile);
   CuAssertIntEquals(tc, 2, spy.numfileCreateCalls);
   CuAssertIntEquals(tc, 2, apx_fileManager_getNumLocalFiles(&manager));
   CuAssertTrue(tc, !spy.lastFile->isRemoteFile);
   CuAssertTrue(tc, !spy.lastFile->isOpen);
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
   apx_fileManagerEventListenerSpy_t spy;
   apx_fileManager_t manager;
   apx_fileManagerEventListenerSpy_create(&spy);

   memset(&listener, 0, sizeof(listener));
   listener.arg = &spy;
   listener.fileCreate = apx_fileManagerEventListenerSpy_fileCreate;

   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_registerEventListener(&manager, &listener);
   rmf_fileInfo_create(&info, "test.apx", 0x10000, 100, RMF_FILE_TYPE_FIXED);
   msgLen = rmf_packHeader(&buffer[0], sizeof(buffer), RMF_CMD_START_ADDR, false);
   msgLen += rmf_serialize_cmdFileInfo(&buffer[msgLen], sizeof(buffer)-msgLen, &info);
   CuAssertIntEquals(tc, msgLen, apx_fileManager_processMessage(&manager, &buffer[0], msgLen));
   CuAssertIntEquals(tc, 1, spy.numfileCreateCalls);
   CuAssertTrue(tc, spy.lastFile->isRemoteFile);
   CuAssertTrue(tc, !spy.lastFile->isOpen);
   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_openRemoteFile_sendMessage(CuTest* tc)
{
   apx_fileManager_t manager;
   uint8_t buffer[100];
   rmf_fileInfo_t info;
   apx_file2_t *file;
   int32_t msgLen;
   apx_transmitHandlerSpy_t spy;
   apx_transmitHandler_t handler;
   adt_bytearray_t *array;
   const uint8_t *data;
   rmf_msg_t msg;
   rmf_cmdOpenFile_t cmd;

   memset(&handler, 0, sizeof(handler));
   handler.getSendBuffer = apx_transmitHandlerSpy_getSendBuffer;
   handler.send = apx_transmitHandlerSpy_send;
   handler.arg = &spy;
   apx_transmitHandlerSpy_create(&spy);
   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_setTransmitHandler(&manager, &handler);
   rmf_fileInfo_create(&info, "test.apx", 0x10000, 100, RMF_FILE_TYPE_FIXED);
   msgLen = rmf_packHeader(&buffer[0], sizeof(buffer), RMF_CMD_START_ADDR, false);
   msgLen += rmf_serialize_cmdFileInfo(&buffer[msgLen], sizeof(buffer)-msgLen, &info);

   CuAssertIntEquals(tc, msgLen, apx_fileManager_processMessage(&manager, &buffer[0], msgLen));

   file = apx_fileMap_findByName(&manager.remote.remoteFileMap, "test.apx");
   CuAssertPtrNotNull(tc, file);
   CuAssertIntEquals(tc, 0, apx_transmitHandlerSpy_length(&spy));
   CuAssertIntEquals(tc, 0, apx_fileManager_openRemoteFile(&manager, file->fileInfo.address, (void*) 0));
   CuAssertIntEquals(tc, 1, apx_fileManager_numPendingMessages(&manager));
   apx_fileManager_run(&manager);
   CuAssertIntEquals(tc, 1, apx_transmitHandlerSpy_length(&spy));
   array = apx_transmitHandlerSpy_next(&spy);
   CuAssertPtrNotNull(tc, array);
   data = adt_bytearray_data(array);
   CuAssertIntEquals(tc, RMF_CMD_TYPE_LEN+RMF_CMD_FILE_OPEN_LEN, adt_bytearray_length(array));
   rmf_unpackMsg(data, adt_bytearray_length(array), &msg);
   CuAssertUIntEquals(tc, RMF_CMD_START_ADDR, msg.address);
   CuAssertIntEquals(tc, 8, rmf_deserialize_cmdOpenFile(msg.data, msg.dataLen, &cmd));
   CuAssertUIntEquals(tc, info.address, cmd.address);

   adt_bytearray_delete(array);
   apx_fileManager_destroy(&manager);
   apx_transmitHandlerSpy_destroy(&spy);
}

static void test_apx_fileManager_openRemoteFile_setOpenFlag(CuTest* tc)
{
   apx_fileManager_t manager;
   uint8_t buffer[100];
   rmf_fileInfo_t info;
   int32_t msgLen;
   apx_file2_t *file;

   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   rmf_fileInfo_create(&info, "test.apx", 0x10000, 100, RMF_FILE_TYPE_FIXED);
   msgLen = rmf_packHeader(&buffer[0], sizeof(buffer), RMF_CMD_START_ADDR, false);
   msgLen += rmf_serialize_cmdFileInfo(&buffer[msgLen], sizeof(buffer)-msgLen, &info);
   CuAssertIntEquals(tc, msgLen, apx_fileManager_processMessage(&manager, &buffer[0], msgLen));

   file = apx_fileMap_findByName(&manager.remote.remoteFileMap, "test.apx");
   CuAssertTrue(tc, file->isOpen == false);
   CuAssertIntEquals(tc, 0, apx_fileManager_openRemoteFile(&manager, file->fileInfo.address, (void*) 0));
   CuAssertTrue(tc, file->isOpen == true);

   apx_fileManager_destroy(&manager);
}

static void test_apx_fileManager_openRemoteFile_processRequest_fixedFileNoReadHandler(CuTest* tc)
{

   apx_fileManager_t manager;
   uint8_t requestBuffer[100];
   rmf_fileInfo_t info;
   apx_file2_t *localFile;
   int32_t msgLen;
   apx_transmitHandlerSpy_t spy;
   apx_transmitHandler_t handler;
   adt_bytearray_t *array;
   const uint8_t *data;
   rmf_msg_t msg;
   rmf_cmdOpenFile_t cmd;
   const uint32_t fileAddress = 0x10000;
   uint32_t unpackedAddress;

   memset(&handler, 0, sizeof(handler));
   handler.getSendBuffer = apx_transmitHandlerSpy_getSendBuffer;
   handler.send = apx_transmitHandlerSpy_send;
   handler.arg = &spy;

   apx_transmitHandlerSpy_create(&spy);

   apx_fileManager_create(&manager, APX_FILEMANAGER_SERVER_MODE, CONNECTION_ID_DEFAULT);
   apx_fileManager_setTransmitHandler(&manager, &handler);

   rmf_fileInfo_create(&info, "test.apx", fileAddress, 100, RMF_FILE_TYPE_FIXED);
   localFile = apx_file2_newLocal(APX_DEFINITION_FILE, &info, NULL);
   apx_fileManager_attachLocalFile(&manager, localFile);
   msgLen = rmf_packHeader(&requestBuffer[0], sizeof(requestBuffer), RMF_CMD_START_ADDR, false);
   cmd.address = fileAddress;
   msgLen += rmf_serialize_cmdOpenFile(&requestBuffer[msgLen], sizeof(requestBuffer)-msgLen, &cmd);

   CuAssertIntEquals(tc, msgLen, apx_fileManager_processMessage(&manager, &requestBuffer[0], msgLen));
   CuAssertIntEquals(tc, 1, apx_fileManager_numPendingMessages(&manager));
   CuAssertIntEquals(tc, 0, apx_transmitHandlerSpy_length(&spy));
   apx_fileManager_run(&manager);

   CuAssertIntEquals(tc, 1, apx_transmitHandlerSpy_length(&spy));
   array = apx_transmitHandlerSpy_next(&spy);
   CuAssertPtrNotNull(tc, array);
   msgLen = RMF_CMD_HEADER_LEN+RMF_ERROR_INVALID_READ_HANDLER_LEN;
   CuAssertIntEquals(tc, msgLen, adt_bytearray_length(array));
   data = adt_bytearray_data(array);
   rmf_unpackMsg(data, msgLen, &msg);
   CuAssertUIntEquals(tc, RMF_CMD_START_ADDR, msg.address);
   rmf_deserialize_errorInvalidReadHandler(msg.data, msg.dataLen, &unpackedAddress);
   CuAssertUIntEquals(tc, fileAddress, unpackedAddress);

   adt_bytearray_delete(array);
   apx_fileManager_destroy(&manager);
   apx_transmitHandlerSpy_destroy(&spy);

}

static void test_apx_fileManager_openRemoteFile_processRequest_fixedFileFromNodeData(CuTest* tc)
{

}
