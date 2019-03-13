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
#define APX_VM_VERSION                2u
#define APX_OPCODE_SIZE               1u //sizeof(uint8_t)

#define APX_OPCODE_NOP                0u

#define APX_OPCODE_PACK_U8            1u
#define APX_OPCODE_PACK_U16           2u
#define APX_OPCODE_PACK_U32           3u
#define APX_OPCODE_PACK_S8            4u
#define APX_OPCODE_PACK_S16           5u
#define APX_OPCODE_PACK_S32           6u
#define APX_OPCODE_PACK_STR           7u
#define APX_OPCODE_PACK_U8AR          8u  //bytes 0-1: array_length (uint16_le)
#define APX_OPCODE_PACK_S8AR          9u  //bytes 0-1: array_length (uint16_le)
#define APX_OPCODE_PACK_U8ARL         10u //bytes 0-3: array_length (uint32_le), reserved only will not be implemented until later
#define APX_OPCODE_PACK_S8ARL         11u //bytes 0-3: array_length (uint32_le), reserved only will not be implemented until later
#define APX_OPCODE_PACK_BOOL          12u

//reserved for future data types
#define APX_OPCODE_UNPACK_U8          64u
#define APX_OPCODE_UNPACK_U16         65u
#define APX_OPCODE_UNPACK_U32         66u
#define APX_OPCODE_UNPACK_S8          67u
#define APX_OPCODE_UNPACK_S16         68u
#define APX_OPCODE_UNPACK_S32         69u
#define APX_OPCODE_UNPACK_STR         70u
#define APX_OPCODE_UNPACK_U8AR        71u  //bytes 0-1: array_length (uint16_le)
#define APX_OPCODE_UNPACK_S8AR        72u  //bytes 0-1: array_length (uint16_le)
#define APX_OPCODE_UNPACK_U8ARL       73u //bytes 0-3: array_length (uint32_le), reserved only will not be implemented until later
#define APX_OPCODE_UNPACK_S8ARL       74u //bytes 0-3: array_length (uint32_le), reserved only will not be implemented until later
#define APX_OPCODE_UNPACK_BOOL        75u
//reserved for future data types
#define APX_OPCODE_PACK_PROG          192u
#define APX_OPCODE_UNPACK_PROG        193u
#define APX_OPCODE_RECORD_ENTER       194u
#define APX_OPCODE_RECORD_SELECT      195u //bytes 0-1: element_id (uint16_le)
#define APX_OPCODE_RECORD_LEAVE       196u
#define APX_OPCODE_ARRAY_ENTER        197u //bytes 0-1: array_length (uint16_le)
#define APX_OPCODE_U8DYNARRAY_ENTER   198u //length of array is found as an uint8 in port data
#define APX_OPCODE_U16DYNARRAY_ENTER  199u //length of array is found as an uint16_le in port data
#define APX_OPCODE_U32DYNARRAY_ENTER  200u //length of array is found as an uint32_le in port data
#define APX_OPCODE_ARRAY_NEXT         201u


#define UINT8_SIZE   1u
#define UINT16_SIZE  2u
#define UINT32_SIZE  4u
#define UINT64_SIZE  8u
#define SINT8_SIZE   1u
#define SINT16_SIZE  2u
#define SINT32_SIZE  4u
#define SINT64_SIZE  8u
#define BOOL_SIZE    sizeof(bool)

#define APX_INST_PACK_PROG_SIZE         7u //bytes 0-1: VM_VERSION(uint16_le), byte2: program_type, bytes 3-6: expected_data_length (uint32_le)
#define APX_INST_UNPACK_PROG_SIZE       7u //bytes 0-1: VM_VERSION(uint16_le), byte2: program_type, bytes 3-6: expected_data_length (uint32_le)
#define APX_INST_PACK_U8_SIZE           1u
#define APX_INST_PACK_U16_SIZE          1u
#define APX_INST_PACK_U32_SIZE          1u
#define APX_INST_PACK_S8_SIZE           1u
#define APX_INST_PACK_S16_SIZE          1u
#define APX_INST_PACK_S32_SIZE          1u
#define APX_INST_PACK_STR_SIZE          3u
#define APX_INST_PACK_U8AR_SIZE         3u
#define APX_INST_PACK_S8AR_SIZE         3u
#define APX_INST_UNPACK_U8_SIZE         1u
#define APX_INST_UNPACK_U16_SIZE        1u
#define APX_INST_UNPACK_U32_SIZE        1u
#define APX_INST_UNPACK_S8_SIZE         1u
#define APX_INST_UNPACK_S16_SIZE        1u
#define APX_INST_UNPACK_S32_SIZE        1u
#define APX_INST_UNPACK_STR_SIZE        3u
#define APX_INST_UNPACK_U8AR_SIZE       3u
#define APX_INST_UNPACK_S8AR_SIZE       3u
#define APX_INST_RECORD_ENTER_SIZE      1u
#define APX_INST_RECORD_SELECT_SIZE     3u
#define APX_INST_RECORD_LEAVE_SIZE      1u
#define APX_INST_ARRAY_ENTER_SIZE       3u
#define APX_INST_DYNARRAY_ENTER_SIZE    1u
#define APX_INST_ARRAY_NEXT_SIZE        1u

#define APX_MAX_INST_PACK_SIZE          3u
#define APX_MAX_INST_UNPACK_SIZE        3u
//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////


#endif //APX_VM_DEFS_H
