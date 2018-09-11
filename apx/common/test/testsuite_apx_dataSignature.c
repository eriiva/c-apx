/*****************************************************************************
* \file      testsuite_apx_dataSignature.c
* \author    Conny Gustafsson
* \date      2017-02-20
* \brief     Unit tests for apx_dataSignature_t
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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "apx_dataSignature.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_dataSignature_create(CuTest* tc);
static void test_apx_dataSignature_uint8(CuTest* tc);
static void test_apx_dataSignature_uint16(CuTest* tc);
static void test_apx_dataSignature_uint16(CuTest* tc);
static void test_apx_dataSignature_uint32(CuTest* tc);
static void test_apx_dataSignature_record(CuTest* tc);
static void test_apx_dataSignature_string(CuTest* tc);
static void test_apx_dataSignature_typeReferenceById(CuTest *tc);
static void test_apx_dataSignature_typeReferenceByIdWithError(CuTest *tc);
static void test_apx_dataSignature_typeReferenceByName(CuTest *tc);
static void test_apx_dataSignature_typeReferenceByNameWithError(CuTest *tc);
static void test_apx_dataSignature_recordWithError(CuTest *tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testsuite_apx_dataSignature(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_dataSignature_create);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_uint8);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_uint16);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_uint32);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_string);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_record);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_typeReferenceById);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_typeReferenceByIdWithError);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_typeReferenceByName);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_typeReferenceByNameWithError);
   SUITE_ADD_TEST(suite, test_apx_dataSignature_recordWithError);

   return suite;
}
//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_dataSignature_create(CuTest* tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_dataSignature_create(&dsg, NULL));
   CuAssertPtrEquals(tc, NULL, dsg.raw);
}

static void test_apx_dataSignature_uint8(CuTest* tc)
{
   apx_dataSignature_t *pSignature;

   //base type
   pSignature = apx_dataSignature_new("C");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT8,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertIntEquals(tc,0,pSignature->dataElement->min.s32);
   CuAssertIntEquals(tc,0,pSignature->dataElement->max.s32);
   CuAssertUIntEquals(tc,sizeof(uint8_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //limit
   pSignature = apx_dataSignature_new("C(0,7)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT8,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,7,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint8_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array
   pSignature = apx_dataSignature_new("C[8]");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT8,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,8,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,sizeof(uint8_t)*8, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array with limit
   pSignature = apx_dataSignature_new("C[8](0,7)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT8,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,8,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,7,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint8_t)*8, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

}

static void test_apx_dataSignature_uint16(CuTest* tc)
{
   apx_dataSignature_t *pSignature;

   //base type
   pSignature = apx_dataSignature_new("S");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT16,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint16_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //limit
   pSignature = apx_dataSignature_new("S(0,512)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT16,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,512,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint16_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array
   pSignature = apx_dataSignature_new("S[16]");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT16,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,16,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint16_t)*16, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array+limit
   pSignature = apx_dataSignature_new("S[16](0,512)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT16,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,16,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,512,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint16_t)*16, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

}

static void test_apx_dataSignature_uint32(CuTest* tc)
{
   apx_dataSignature_t *pSignature;

   //base type
   pSignature = apx_dataSignature_new("L");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT32,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint32_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //limit
   pSignature = apx_dataSignature_new("L(0,999999)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT32,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,999999,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint32_t), apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array
   pSignature = apx_dataSignature_new("L[2]");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT32,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,2,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint32_t)*2, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

   //array+limit
   pSignature = apx_dataSignature_new("L[2](0,999999)");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_UINT32,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,2,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,999999,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc,sizeof(uint32_t)*2, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);
}

static void test_apx_dataSignature_record(CuTest* tc)
{
   apx_dataSignature_t *pSignature;

   //base type
   pSignature = apx_dataSignature_new("{\"NodeId\"C\"DTCId\"S\"FailT\"C\"RqstData\"C(0,3)}");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_RECORD,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertPtrNotNull(tc,pSignature->dataElement->childElements);
   apx_dataSignature_calcPackLen(pSignature);
   CuAssertUIntEquals(tc, 5, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);



   pSignature = apx_dataSignature_new("{\"NodeId\"C\"HWPartNo\"a[9]\"HWRevNo\"a[4]\"HWSerialNo\"a[9]\"SWPartNo\"a[9]\"SWRevNo\"a[4]}");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_RECORD,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertPtrNotNull(tc,pSignature->dataElement->childElements);
   apx_dataSignature_calcPackLen(pSignature);
   CuAssertUIntEquals(tc, 36, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

}

static void test_apx_dataSignature_string(CuTest* tc)
{
   apx_dataSignature_t *pSignature;
   pSignature = apx_dataSignature_new("a[10]");
   CuAssertPtrNotNull(tc, pSignature);
   CuAssertIntEquals(tc,APX_BASE_TYPE_STRING,pSignature->dataElement->baseType);
   CuAssertPtrEquals(tc,NULL,pSignature->dataElement->name);
   CuAssertUIntEquals(tc,10,pSignature->dataElement->arrayLen);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->min.u32);
   CuAssertUIntEquals(tc,0,pSignature->dataElement->max.u32);
   CuAssertUIntEquals(tc, 10, apx_dataSignature_calcPackLen(pSignature));
   apx_dataSignature_delete(pSignature);

}

static void test_apx_dataSignature_typeReferenceById(CuTest *tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_dataSignature_create(&dsg, "T[0]"));
   apx_dataSignature_destroy(&dsg);
}

static void test_apx_dataSignature_typeReferenceByIdWithError(CuTest *tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_UNMATCHED_BRACKET_ERROR, apx_dataSignature_create(&dsg, "T[0"));
   CuAssertIntEquals(tc, APX_INVALID_TYPE_REF_ERROR, apx_dataSignature_create(&dsg, "T[]"));
   CuAssertIntEquals(tc, APX_INVALID_TYPE_REF_ERROR, apx_dataSignature_create(&dsg, "T[garbage]"));
   CuAssertIntEquals(tc, APX_INVALID_TYPE_REF_ERROR, apx_dataSignature_create(&dsg, "T[0garbage]"));
}

static void test_apx_dataSignature_typeReferenceByName(CuTest *tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_NO_ERROR, apx_dataSignature_create(&dsg, "T[\"type_T\"]"));
   apx_dataSignature_destroy(&dsg);
}

static void test_apx_dataSignature_typeReferenceByNameWithError(CuTest *tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_UNMATCHED_BRACKET_ERROR, apx_dataSignature_create(&dsg, "T[\"type_T"));
   CuAssertIntEquals(tc, APX_UNMATCHED_STRING_ERROR, apx_dataSignature_create(&dsg, "T[\"type_T]"));
}

static void test_apx_dataSignature_recordWithError(CuTest *tc)
{
   apx_dataSignature_t dsg;
   CuAssertIntEquals(tc, APX_UNMATCHED_BRACE_ERROR, apx_dataSignature_create(&dsg, "{\"UserId\"S"));

}


