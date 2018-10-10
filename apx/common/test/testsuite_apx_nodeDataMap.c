/*****************************************************************************
* \file      testsuite_apx_nodeDataMap.c
* \author    Conny Gustafsson
* \date      2018-10-08
* \brief     Unit tests for apx_nodeDataMap_t
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
#include "apx_nodeDataMap.h"
#include "apx_error.h"
#include "apx_parser.h"
#include "apx_portInfo.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataMap_create_serverMap(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testSuite_apx_nodeDataMap(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_apx_nodeDataMap_create_serverMap);


   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_apx_nodeDataMap_create_serverMap(CuTest* tc)
{
   const char *apx_text =
         "APX/1.2\n"
         "N\"TestNode\"\n"
         "P\"Name\"a[8]\n"
         "P\"Id\"L\n";
   apx_parser_t parser;
   apx_node_t *node;
   apx_nodeData_t *nodeData;
   apx_nodeDataMap_t *map;
   apx_parser_create(&parser);
   node = apx_parser_parseString(&parser, apx_text);
   CuAssertPtrNotNull(tc, node);
   apx_parser_clearNodes(&parser);
   nodeData = apx_nodeData_new((uint32_t) strlen(apx_text));
   CuAssertPtrNotNull(tc, nodeData);
   apx_nodeData_setNode(nodeData, node);
   map = apx_nodeDataMap_new(nodeData, APX_SERVER_MODE);
   CuAssertPtrNotNull(tc, map);
   int32_t portLen = adt_ary_length(&map->providePortInfoList);
   apx_portInfo_t *portInfo;
   CuAssertUIntEquals(tc, 2, portLen);
   portInfo = (apx_portInfo_t*) adt_ary_value(&map->providePortInfoList, 0);
   CuAssertPtrNotNull(tc, portInfo);
   CuAssertIntEquals(tc, 8, portInfo->dataSize);
   CuAssertPtrEquals(tc, nodeData, portInfo->nodedata);
   CuAssertIntEquals(tc, 0, portInfo->offset);
   CuAssertIntEquals(tc, 0, portInfo->portIndex);
   portInfo = (apx_portInfo_t*) adt_ary_value(&map->providePortInfoList, 1);
   CuAssertPtrNotNull(tc, portInfo);
   CuAssertUIntEquals(tc, 4, portInfo->dataSize);
   CuAssertPtrEquals(tc, nodeData, portInfo->nodedata);
   CuAssertIntEquals(tc, 8, portInfo->offset);
   CuAssertIntEquals(tc, 1, portInfo->portIndex);
   CuAssertPtrEquals(tc, 0, map->requireBytePortMap);
   CuAssertPtrNotNull(tc, map->provideBytePortMap);
   CuAssertUIntEquals(tc, 12, map->provideBytePortMap->mapLen);
   apx_nodeDataMap_delete(map);
   apx_nodeData_delete(nodeData);
   apx_parser_destroy(&parser);
}

;
