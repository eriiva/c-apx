/*****************************************************************************
* \file      apx_eventRecorderSrvRmf.c
* \author    Conny Gustafsson
* \date      2018-08-12
* \brief     Event recorder attached to a single instance of apx_fileManager_t
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
#include "apx_eventRecorderSrvRmf.h"
#include <assert.h>
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderSrvRmf_create(apx_eventRecorderSrvRmf_t *self, struct apx_fileManager_tag *fileManager, int32_t minimumUpdateTime)
{
   if (self != 0)
   {
      self->fileManager = fileManager;
      if (minimumUpdateTime==0)
      {
         minimumUpdateTime = 1;
      }
      self->minimumUpdateTime = minimumUpdateTime;
   }
}

void apx_eventRecorderSrvRmf_destroy(apx_eventRecorderSrvRmf_t *self)
{
   if (self != 0)
   {

   }
}

apx_eventRecorderSrvRmf_t *apx_eventRecorderSrvRmf_new(struct apx_fileManager_tag *fileManager, int32_t minimumUpdateTime)
{
   apx_eventRecorderSrvRmf_t *self = (apx_eventRecorderSrvRmf_t*) malloc(sizeof(apx_eventRecorderSrvRmf_t));
   if(self != 0)
   {
      apx_eventRecorderSrvRmf_create(self, fileManager, minimumUpdateTime);
   }
   return self;
}

void apx_eventRecorderSrvRmf_delete(apx_eventRecorderSrvRmf_t *self)
{
   if(self != 0)
   {
      apx_eventRecorderSrvRmf_destroy(self);
      free(self);
   }
}

void apx_eventRecorderSrvRmf_vdelete(void *arg)
{
   apx_eventRecorderSrvRmf_delete((apx_eventRecorderSrvRmf_t*) arg);
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


