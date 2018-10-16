/*****************************************************************************
* \file      testsuite_apx_eventLoop.c
* \author    Conny Gustafsson
* \date      2018-10-15
* \brief     Unit tests for APX event loop
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
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "CuTest.h"
#include "apx_eventLoop.h"
#include "apx_fileManager.h"

#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_eventLoop_newConnection(CuTest* tc);

static void handlerReset(void);
static void onConnected(void *arg, struct apx_fileManager_tag *fileManager);
//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////
static uint32_t m_onConnectedCount;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_eventLoop(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_eventLoop_newConnection);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_eventLoop_newConnection(CuTest* tc)
{
   apx_fileManager_t fileManager;
   apx_eventLoop_t *loop = apx_eventLoop_new();
   CuAssertPtrNotNull(tc, loop);
   CuAssertIntEquals(tc, 0, apx_fileManager_create(&fileManager, APX_FILEMANAGER_SERVER_MODE, 0));
   handlerReset();
   apx_eventLoop_emitConnected(loop, onConnected, 0, &fileManager);
   CuAssertUIntEquals(tc, 0, m_onConnectedCount);
   apx_eventLoop_run(loop);
   CuAssertUIntEquals(tc, 1, m_onConnectedCount);

   apx_fileManager_destroy(&fileManager);
   apx_eventLoop_delete(loop);

}


static void handlerReset(void)
{
   m_onConnectedCount = 0;
}

static void onConnected(void *arg, struct apx_fileManager_tag *fileManager)
{
   m_onConnectedCount++;
}
