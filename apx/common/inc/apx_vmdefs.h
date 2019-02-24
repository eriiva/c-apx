/*****************************************************************************
* \file      apx_vmDefs.h
* \author    Conny Gustafsson
* \date      2019-01-03
* \brief     APX virtual machine shared definitions
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
#ifndef APX_VM_DEFS_H
#define APX_VM_DEFS_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"
//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_VM_VERSION           3u

#define APX_OPCODE_NOP           0u
#define APX_OPCODE_PACK_PROG     1u
#define APX_OPCODE_UNPACK_PROG   2u

#define APX_OPCODE_PACK_U8       3u
#define APX_OPCODE_PACK_U16      4u
#define APX_OPCODE_PACK_U32      5u
#define APX_OPCODE_PACK_S8       6u
#define APX_OPCODE_PACK_S16      7u
#define APX_OPCODE_PACK_S32      8u
#define APX_OPCODE_PACK_STR      9u
#define APX_OPCODE_PACK_U8AR     10u
#define APX_OPCODE_PACK_U16AR    11u
#define APX_OPCODE_PACK_U32AR    12u
#define APX_OPCODE_PACK_S8AR     13u
#define APX_OPCODE_PACK_S16AR    14u
#define APX_OPCODE_PACK_S32AR    15u
//reserved for future data types
#define APX_OPCODE_UNPACK_U8     64u
#define APX_OPCODE_UNPACK_U16    65u
#define APX_OPCODE_UNPACK_U32    66u
#define APX_OPCODE_UNPACK_S8     67u
#define APX_OPCODE_UNPACK_S16    68u
#define APX_OPCODE_UNPACK_S32    69u
#define APX_OPCODE_UNPACK_STR    70u
#define APX_OPCODE_UNPACK_U8AR   71u
#define APX_OPCODE_UNPACK_U16AR  72u
#define APX_OPCODE_UNPACK_U32AR  73u
#define APX_OPCODE_UNPACK_S8AR   74u
#define APX_OPCODE_UNPACK_S16AR  75u
#define APX_OPCODE_UNPACK_S32AR  76u
//reserved for future data types
#define APX_OPCODE_RECORD_ENTER  128u
#define APX_OPCODE_RECORD_SELECT 129u
#define APX_OPCODE_RECORD_LEAVE  130u
#define APX_OPCODE_ARRAY_ENTER   131u
#define APX_OPCODE_ARRAY_LEAVE   132u

#define APX_OPCODE_SIZE          1u //sizeof(uint8_t)

#define UINT8_SIZE   1u
#define UINT16_SIZE  2u
#define UINT32_SIZE  4u
#define UINT64_SIZE  8u
#define SINT8_SIZE   1u
#define SINT16_SIZE  2u
#define SINT32_SIZE  4u
#define SINT64_SIZE  8u

#define APX_INST_PACK_PROG_SIZE        7u
#define APX_INST_UNPACK_PROG_SIZE      7u
#define APX_INST_PACK_U8_SIZE          1u
#define APX_INST_PACK_U16_SIZE         1u
#define APX_INST_PACK_U32_SIZE         1u
#define APX_INST_PACK_S8_SIZE          1u
#define APX_INST_PACK_S16_SIZE         1u
#define APX_INST_PACK_S32_SIZE         1u
#define APX_INST_PACK_STR_SIZE         3u
#define APX_INST_PACK_U8AR_SIZE        3u
#define APX_INST_PACK_U16AR_SIZE       3u
#define APX_INST_PACK_U32AR_SIZE       3u
#define APX_INST_PACK_S8AR_SIZE        3u
#define APX_INST_PACK_S16AR_SIZE       3u
#define APX_INST_PACK_S32AR_SIZE       3u
#define APX_INST_UNPACK_U8_SIZE        1u
#define APX_INST_UNPACK_U16_SIZE       1u
#define APX_INST_UNPACK_U32_SIZE       1u
#define APX_INST_UNPACK_S8_SIZE        1u
#define APX_INST_UNPACK_S16_SIZE       1u
#define APX_INST_UNPACK_S32_SIZE       1u
#define APX_INST_UNPACK_STR_SIZE       3u
#define APX_INST_UNPACK_U8AR_SIZE      3u
#define APX_INST_UNPACK_U16AR_SIZE     3u
#define APX_INST_UNPACK_U32AR_SIZE     3u
#define APX_INST_UNPACK_S8AR_SIZE      3u
#define APX_INST_UNPACK_S16AR_SIZE     3u
#define APX_INST_UNPACK_S32AR_SIZE     3u
#define APX_INST_RECORD_ENTER_SIZE     1u
#define APX_INST_RECORD_SELECT_SIZE    3u
#define APX_INST_RECORD_LEAVE_SIZE     1u
#define APX_INST_ARRAY_ENTER_SIZE      1u
#define APX_INST_ARRAY_LEAVE_SIZE      1u

#define APX_MAX_INST_PACK_SIZE         3u
#define APX_MAX_INST_UNPACK_SIZE       3u
//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////


#endif //APX_VM_DEFS_H
