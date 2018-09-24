//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include "CuTest.h"
#include "filestream.h"
#include "adt_bytearray.h"
#include "sha256.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#define APX_TEST_DATA_PATH "..\\..\\..\\apx\\common\\test\\data\\"
#else
#define APX_TEST_DATA_PATH  "../../../apx/common/test/data/"
#endif

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void fstream_open(void *arg);
static void fstream_write(void *arg, const uint8_t *pChunk, uint32_t chunkLen);
static void test_calc_test1(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
adt_bytearray_t m_buf;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testsuite_sha256(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_calc_test1);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void fstream_open(void *arg)
{
   adt_bytearray_create(&m_buf, 0);
}

static void fstream_write(void *arg, const uint8_t *pChunk, uint32_t chunkLen)
{
   adt_bytearray_append(&m_buf, pChunk, chunkLen);
}

static void test_calc_test1(CuTest* tc)
{
   ifstream_handler_t handler;
   ifstream_t stream;
   uint8_t hash[32];
   memset(&handler, 0, sizeof(handler));
   handler.open = fstream_open;
   handler.write = fstream_write;
   ifstream_create(&stream, &handler);
   CuAssertIntEquals(tc, 0, ifstream_readBinaryFile(&stream, APX_TEST_DATA_PATH "test1.apx"));
   memset(hash, 0, sizeof(hash));
   sha256_calc(hash, (void*)adt_bytearray_data(&m_buf), (size_t) adt_bytearray_length(&m_buf));
   CuAssertUIntEquals(tc, 0xb2, hash[0]);
   CuAssertUIntEquals(tc, 0xb6, hash[1]);
   CuAssertUIntEquals(tc, 0xb1, hash[2]);
   CuAssertUIntEquals(tc, 0x36, hash[3]);
   CuAssertUIntEquals(tc, 0x37, hash[4]);
   CuAssertUIntEquals(tc, 0xe5, hash[5]);
   CuAssertUIntEquals(tc, 0xe2, hash[6]);
   CuAssertUIntEquals(tc, 0xed, hash[7]);
   CuAssertUIntEquals(tc, 0x14, hash[8]);
   CuAssertUIntEquals(tc, 0x3d, hash[9]);
   CuAssertUIntEquals(tc, 0x79, hash[10]);
   CuAssertUIntEquals(tc, 0x25, hash[11]);
   CuAssertUIntEquals(tc, 0xfc, hash[12]);
   CuAssertUIntEquals(tc, 0x6c, hash[13]);
   CuAssertUIntEquals(tc, 0x2f, hash[14]);
   CuAssertUIntEquals(tc, 0x44, hash[15]);
   CuAssertUIntEquals(tc, 0xd3, hash[16]);
   CuAssertUIntEquals(tc, 0x9f, hash[17]);
   CuAssertUIntEquals(tc, 0xba, hash[18]);
   CuAssertUIntEquals(tc, 0xbd, hash[19]);
   CuAssertUIntEquals(tc, 0x8c, hash[20]);
   CuAssertUIntEquals(tc, 0x22, hash[21]);
   CuAssertUIntEquals(tc, 0x2f, hash[22]);
   CuAssertUIntEquals(tc, 0x19, hash[23]);
   CuAssertUIntEquals(tc, 0x8f, hash[24]);
   CuAssertUIntEquals(tc, 0x2c, hash[25]);
   CuAssertUIntEquals(tc, 0x43, hash[26]);
   CuAssertUIntEquals(tc, 0x73, hash[27]);
   CuAssertUIntEquals(tc, 0x08, hash[28]);
   CuAssertUIntEquals(tc, 0x51, hash[29]);
   CuAssertUIntEquals(tc, 0x19, hash[30]);
   CuAssertUIntEquals(tc, 0xa7, hash[31]);
   adt_bytearray_destroy(&m_buf);
}

