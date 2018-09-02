/*****************************************************************************
* \file      apx_file2.h
* \author    Conny Gustafsson
* \date      2018-08-30
* \brief     Improved version of apx_file
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
#ifndef APX_FILE2_H
#define APX_FILE2_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include "apx_types.h"
#include "rmf.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef int8_t (apx_file_read_func)(void *arg, const rmf_fileInfo_t *fileInfo, uint8_t *dest, uint32_t offset, uint32_t len);
typedef int8_t (apx_file_write_func)(void *arg, const rmf_fileInfo_t *fileInfo, const uint8_t *src, uint32_t offset, uint32_t len);
typedef const char* (apx_file_basename_func)(void *arg);

#define APX_FILE_RESULT_FAIL -1
#define APX_FILE_RESULT_OK    0

typedef struct apx_file_handler_tag
{
   void *arg;
   apx_file_read_func *read;
   apx_file_write_func *write;
   apx_file_basename_func *basename;
}apx_file_handler_t;

typedef struct apx_file2_tag
{
   bool isRemoteFile;
   bool isOpen;
   uint8_t fileType;
   rmf_fileInfo_t fileInfo;
   apx_file_handler_t handler;
}apx_file2_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_file2_create(apx_file2_t *self, uint8_t fileType, bool isRemoteFile, const rmf_fileInfo_t *fileInfo, const apx_file_handler_t *handler);
#ifndef APX_EMBEDDED
void apx_file2_destroy(apx_file2_t *self);
apx_file2_t *apx_file2_new(uint8_t fileType, bool isRemoteFile, const rmf_fileInfo_t *fileInfo, const apx_file_handler_t *handler);
# define apx_file2_newLocal(fileType, fileInfo, handler) apx_file2_new(fileType, false, fileInfo, handler);
# define apx_file2_newRemote(fileType, fileInfo, handler) apx_file2_new(fileType, true, fileInfo, handler);
void apx_file2_delete(apx_file2_t *self);
void apx_file2_vdelete(void *arg);
#endif
const char *apx_file2_basename(const apx_file2_t *self);
void apx_file2_open(apx_file2_t *self);
void apx_file2_close(apx_file2_t *self);
int8_t apx_file2_read(apx_file2_t *self, uint8_t *pDest, uint32_t offset, uint32_t length); //returns APX_FILE_RESULT_XXX
int8_t apx_file2_write(apx_file2_t *self, const uint8_t *pSrc, uint32_t offset, uint32_t length); //returns APX_FILE_RESULT_XXX
bool apx_file2_hasReadHandler(apx_file2_t *self);
bool apx_file2_hasWriteHandler(apx_file2_t *self);

#endif //APX_FILE2_H

