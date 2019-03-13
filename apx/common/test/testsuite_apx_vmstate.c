/*****************************************************************************
* \file      testsuite_apx_vmstate.c
* \author    Conny Gustafsson
* \date      2019-03-10
* \brief     Description
*
* Copyright (c) 2019 Conny Gustafsson
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
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_vmstate.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_apx_vmstate_packU8(CuTest* tc);
static void test_apx_apx_vmstate_packU8U8DynArray(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_vmstate(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_apx_vmstate_packU8);
   SUITE_ADD_TEST(suite, test_apx_apx_vmstate_packU8U8DynArray);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_apx_vmstate_packU8(CuTest* tc)
{
   uint8_t data[3];
   memset(&data[0], 0, sizeof(data));
   apx_vmstate_t *st = apx_vmstate_new();
   CuAssertPtrNotNull(tc, st);
   CuAssertIntEquals(tc, APX_MISSING_BUFFER_ERROR, apx_vmstate_packU8(st, (uint8_t) 1u));
   apx_vmstate_setWriteData(st, &data[0], sizeof(data));
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_vmstate_packU8(st, (uint8_t) 1u));
   CuAssertUIntEquals(tc, 1, data[0]);
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_vmstate_packU8(st, (uint8_t) 0x12));
   CuAssertUIntEquals(tc, 0x12, data[1]);
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_vmstate_packU8(st, (uint8_t) 0xff));
   CuAssertUIntEquals(tc, 0xff, data[2]);
   CuAssertIntEquals(tc, APX_BUFFER_BOUNDARY_ERROR, apx_vmstate_packU8(st, (uint8_t) 0));
   apx_vmstate_delete(st);
}

static void test_apx_apx_vmstate_packU8U8DynArray(CuTest* tc)
{
   uint8_t data[9];
   uint8_t values[8]={0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};
   uint8_t verificationData[9];
   memset(&data[0], 0, sizeof(data));
   verificationData[0]=sizeof(values);
   memcpy(&verificationData[1], &values[0], sizeof(values));
   apx_vmstate_t *st = apx_vmstate_new();
   CuAssertPtrNotNull(tc, st);
   CuAssertIntEquals(tc, APX_MISSING_BUFFER_ERROR, apx_vmstate_packU8(st, (uint8_t) 1u));
   apx_vmstate_setWriteData(st, &data[0], sizeof(data));
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_vmstate_packU8U8DynArray(st, &values[0], 8u));
   CuAssertIntEquals(tc, 0, memcmp(data, verificationData, 9));
   apx_vmstate_delete(st);
}

