/*****************************************************************************
* \file      apx_file2.c
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
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "apx_file2.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_file2_set_handler(apx_file2_t *self, const apx_file_handler_t *handler);
//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_file2_create(apx_file2_t *self, uint8_t fileType, bool isRemoteFile, const rmf_fileInfo_t *fileInfo, const apx_file_handler_t *handler)
{
   if (self != 0)
   {
      int8_t result;
      self->fileType = fileType;
      self->isRemoteFile = isRemoteFile;
      self->isOpen = false;

      result = rmf_fileInfo_create(&self->fileInfo, fileInfo->name, fileInfo->address, fileInfo->length, fileInfo->fileType);
      if (result == 0)
      {
         rmf_fileInfo_setDigestData(&self->fileInfo, fileInfo->digestType, fileInfo->digestData, 0);
         apx_file2_set_handler(self, handler);
      }
      return result;
   }
   errno = EINVAL;
   return -1;
}

#ifndef APX_EMBEDDED
void apx_file2_destroy(apx_file2_t *self)
{
   if (self != 0)
   {
      rmf_fileInfo_destroy(&self->fileInfo);
   }
}

apx_file2_t *apx_file2_new(uint8_t fileType, bool isRemoteFile, const rmf_fileInfo_t *fileInfo, const apx_file_handler_t *handler)
{
   apx_file2_t *self = (apx_file2_t*) malloc(sizeof(apx_file2_t));
   if (self != 0)
   {
      int8_t result = apx_file2_create(self, fileType, isRemoteFile, fileInfo, handler);
      if (result != 0)
      {
         free(self);
         self = 0;
      }
   }
   else
   {
      errno = ENOMEM;
   }
   return self;
}

void apx_file2_delete(apx_file2_t *self)
{
   if (self != 0)
   {
      apx_file2_destroy(self);
      free(self);
   }
}

void apx_file2_vdelete(void *arg)
{
   apx_file2_delete((apx_file2_t*) arg);
}

#endif

const char *apx_file2_basename(const apx_file2_t *self)
{
   if ( (self != 0) && (self->handler.basename !=0) )
   {
      return self->handler.basename(self->handler.arg);
   }
   return (char*) 0;
}

void apx_file2_open(apx_file2_t *self)
{
   if (self != 0)
   {
      self->isOpen = true;
   }
}

void apx_file2_close(apx_file2_t *self)
{
   if (self != 0)
   {
      self->isOpen = false;
   }
}

int8_t apx_file2_read(apx_file2_t *self, uint8_t *pDest, uint32_t offset, uint32_t length)
{
   if ( (self !=0) && (self->handler.read != 0) )
   {
      return self->handler.read(self->handler.arg, &self->fileInfo, pDest, offset, length);
   }
   errno = EINVAL;
   return -1;
}

int8_t apx_file2_write(apx_file2_t *self, const uint8_t *pSrc, uint32_t offset, uint32_t length)
{
   if ( (self !=0) && (self->handler.write != 0) )
   {
      return self->handler.write(self->handler.arg, &self->fileInfo, pSrc, offset, length);
   }
   errno = EINVAL;
   return -1;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_file2_set_handler(apx_file2_t *self, const apx_file_handler_t *handler)
{
   if (handler == 0)
   {
      memset(&self->handler, 0, sizeof(apx_file_handler_t));
   }
   else
   {
      memcpy(&self->handler, handler, sizeof(apx_file_handler_t));
   }
}

