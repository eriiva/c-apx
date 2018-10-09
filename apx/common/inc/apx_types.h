#ifndef APX_TYPES_H
#define APX_TYPES_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "apx_cfg.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef int32_t apx_offset_t;
typedef uint32_t apx_size_t; //use uint16_t  to send up to 64KB, use uint32_t for 4GB.
typedef APX_PORT_ID_TYPE apx_portId_t; //uint32_t is default. Use uint16_t for smaller memory footprint
typedef uint8_t apx_portType_t;

typedef struct apx_dataWriteCmd_tag
{
   apx_offset_t offset;
   apx_size_t len;
} apx_dataWriteCmd_t;

#define APX_DATA_WRITE_CMD_SIZE sizeof(apx_dataWriteCmd_t)

#define APX_CONNECTION_TYPE_TEST_SOCKET       0
#define APX_CONNECTION_TYPE_TCP_SOCKET        1
#define APX_CONNECTION_TYPE_LOCAL_SOCKET      2

#define APX_DEBUG_NONE                        0
#define APX_DEBUG_1_PROFILE                   1  // Used to profile node connect performance using timestamped log
#define APX_DEBUG_2_LOW                       2
#define APX_DEBUG_3_MID                       3
#define APX_DEBUG_4_HIGH                      4

#define APX_DEBUG_INFO_MAX_LEN 20

#define APX_UNKNOWN_FILE          0
#define APX_OUTDATA_FILE          1
#define APX_INDATA_FILE           2
#define APX_DEFINITION_FILE       3
#define APX_USER_DATA_FILE        4
#define APX_EVENT_FILE            5

#define APX_MAX_FILE_EXT_LEN      4 //'.xxx'
#define APX_OUTDATA_FILE_EXT      ".out"
#define APX_INDATA_FILE_EXT       ".in"
#define APX_DEFINITION_FILE_EXT   ".apx"
#define APX_EVENT_FILE_EXT        ".event"

#define APX_CHECKSUM_NONE         0u
#define APX_CHECKSUM_SHA256       1u

#define APX_CLIENT_MODE           0u
#define APX_SERVER_MODE           1u

#define APX_CHECKSUMLEN_SHA256    32u

#if defined(_MSC_PLATFORM_TOOLSET) && (_MSC_PLATFORM_TOOLSET<=110)
#include "msc_bool.h"
#else
#include <stdbool.h>
#endif

#ifdef _MSC_VER
#define STRDUP _strdup
#else
#define STRDUP strdup
#endif


#endif //APX_TYPES_H
