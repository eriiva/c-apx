/*****************************************************************************
* \file      apx_dataElement.h
* \author    Conny Gustafsson
* \date      2017-02-20
* \brief     Data element data structure
*
* Copyright (c) 2017-2018 Conny Gustafsson
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
#ifndef APX_DATAELEMENT_H
#define APX_DATAELEMENT_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "adt_ary.h"
#include "dtl_type.h"
#include "apx_error.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_datatype_tag;

#define APX_BASE_TYPE_NONE     -1
#define APX_BASE_TYPE_UINT8    0 //'C' (uint8)
#define APX_BASE_TYPE_UINT16   1 //'S' (uint16)
#define APX_BASE_TYPE_UINT32   2 //'L' (uin32)
#define APX_BASE_TYPE_UINT64   3 //'U' (uint64)
#define APX_BASE_TYPE_SINT8    4 //'c'
#define APX_BASE_TYPE_SINT16   5 //'s'
#define APX_BASE_TYPE_SINT32   6 //'l'
#define APX_BASE_TYPE_SINT64   7 //'u'
#define APX_BASE_TYPE_STRING   8 //'a' (string)
#define APX_BASE_TYPE_RECORD   9 //"{}" (record)
#define APX_BASE_TYPE_REF_ID   10 //type ID
#define APX_BASE_TYPE_REF_NAME 11 //type name
#define APX_BASE_TYPE_REF_PTR  12 //pointer to type (this is achieved only after derived has been called on data signature)

typedef struct apx_dataElement_tag
{
   char *name;
   int8_t baseType;
   uint32_t arrayLen;
   uint32_t packLen;
   union {
      uint32_t u32;
      int32_t  s32;
   }min;
   union {
      uint32_t u32;
      int32_t  s32;
   }max;
   adt_ary_t *childElements; //NULL for all cases except when baseType is exactly == APX_BASE_TYPE_RECORD
   union {
      int32_t id; //used when baseType is APX_BASE_TYPE_REFERENCE_INT
      char *name; //used when baseType is APX_BASE_TYPE_REFERENCE_STR
      struct apx_datatype_tag *ptr; //used when baseType is APX_BASE_TYPE_REF_PTR
   }typeRef;
}apx_dataElement_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_dataElement_t *apx_dataElement_new(int8_t baseType, const char *name);
void apx_dataElement_delete(apx_dataElement_t *self);
void apx_dataElement_vdelete(void *arg);
int8_t apx_dataElement_create(apx_dataElement_t *self, int8_t baseType, const char *name);
void apx_dataElement_destroy(apx_dataElement_t *self);
void apx_dataElement_initRecordType(apx_dataElement_t *self);
uint8_t *apx_dataElement_pack_dv(apx_dataElement_t *self, uint8_t *pBegin, uint8_t *pEnd, dtl_dv_t *dv);

void apx_dataElement_setArrayLen(apx_dataElement_t *self, uint32_t arrayLen);
uint32_t apx_dataElement_getArrayLen(apx_dataElement_t *self);
void apx_dataElement_appendChild(apx_dataElement_t *self, apx_dataElement_t *child);
int32_t apx_dataElement_getNumChild(apx_dataElement_t *self);
apx_dataElement_t *apx_dataElement_getChildAt(apx_dataElement_t *self, int32_t index);
void apx_dataElement_setTypeReferenceId(apx_dataElement_t *self, int32_t typeId);
int32_t apx_dataElement_getTypeReferenceId(apx_dataElement_t *self);
void apx_dataElement_setTypeReferenceName(apx_dataElement_t *self, const char *typeName);
const char *apx_dataElement_getTypeReferenceName(apx_dataElement_t *self);
void apx_dataElement_setTypeReferencePtr(apx_dataElement_t *self, struct apx_datatype_tag *ptr);
struct apx_datatype_tag *apx_dataElement_getTypeReferencePtr(apx_dataElement_t *self);
int32_t apx_dataElement_calcPackLen(apx_dataElement_t *self);
int32_t apx_dataElement_getPackLen(apx_dataElement_t *self);

#endif //APX_DATAELEMENT_H
