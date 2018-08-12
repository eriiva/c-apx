/*****************************************************************************
* \file      apx_eventRecorderSrvRmfMgr.c
* \author    Conny Gustafsson
* \date      2018-08-12
* \brief     Handles multiple instances apx_eventRecorderSrvRmf_t
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
#include <string.h>
#include "apx_eventRecorderSrvRmfMgr.h"
#include "apx_eventRecorderSrvRmf.h"
#include "apx_eventFile.h"
#include "apx_file.h"
#include "apx_fileManager.h"

//temporary include
#include <stdio.h>

#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_eventRecordrSrvRmfMgr_newConnection(void *arg, apx_fileManager_t *fileManager);
static void apx_eventRecordrSrvRmfMgr_disconnected(void *arg, apx_fileManager_t *fileManager);
static void apx_eventRecorderSrvRmfMgr_registerEventFile(apx_eventRecorderSrvRmfMgr_t *self, apx_fileManager_t *fileManager);
static void apx_eventRecorderSrvRmfMgr_registerEventListener(apx_eventRecorderSrvRmfMgr_t *self, apx_fileManager_t *fileManager);
static void apx_eventRecorderSrvRmfMgr_onFileOpen(void *arg, apx_fileManager_t *fileManager, const apx_file_t *file);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderSrvRmfMgr_create(apx_eventRecorderSrvRmfMgr_t *self, int32_t minimumUpdateTime)
{
   if (self != 0)
   {
      self->minimumUpdateTime = minimumUpdateTime;
      self->base.newConnection = apx_eventRecordrSrvRmfMgr_newConnection;
      self->base.disconnected =  apx_eventRecordrSrvRmfMgr_disconnected;
      adt_list_create(&self->instanceList, apx_eventRecorderSrvRmf_vdelete);
   }
}

void apx_eventRecorderSrvRmfMgr_destroy(apx_eventRecorderSrvRmfMgr_t *self)
{

}

apx_eventRecorderSrvRmfMgr_t *apx_eventRecorderSrvRmfMgr_new(int32_t minimumUpdateTime)
{
   apx_eventRecorderSrvRmfMgr_t *self = (apx_eventRecorderSrvRmfMgr_t*) malloc(sizeof(apx_eventRecorderSrvRmfMgr_t));
   if(self != 0)
   {
      apx_eventRecorderSrvRmfMgr_create(self, minimumUpdateTime);
   }
   return self;
}

void apx_eventRecorderSrvRmfMgr_delete(apx_eventRecorderSrvRmfMgr_t *self)
{
   if(self != 0)
   {
      apx_eventRecorderSrvRmfMgr_destroy(self);
      free(self);
   }
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_eventRecordrSrvRmfMgr_newConnection(void *arg, apx_fileManager_t *fileManager)
{
   apx_eventRecorderSrvRmfMgr_t *self = (apx_eventRecorderSrvRmfMgr_t*) arg;
   if ( (self != 0) && (fileManager) )
   {
      apx_eventRecorderSrvRmfMgr_registerEventFile(self, fileManager);
      apx_eventRecorderSrvRmfMgr_registerEventListener(self, fileManager);
   }
}

static void apx_eventRecordrSrvRmfMgr_disconnected(void *arg, apx_fileManager_t *fileManager)
{
   printf("disconnected\n");
}

static void apx_eventRecorderSrvRmfMgr_registerEventFile(apx_eventRecorderSrvRmfMgr_t *self, apx_fileManager_t *fileManager)
{
   apx_file_t *eventFile = apx_eventFile_new();
   if (eventFile != 0)
   {
      apx_fileManager_attachLocalFile(fileManager, eventFile);
      printf("event file created in new fileManager\n");
   }
}

static void apx_eventRecorderSrvRmfMgr_registerEventListener(apx_eventRecorderSrvRmfMgr_t *self, apx_fileManager_t *fileManager)
{
   apx_eventListener_t listener;
   memset(&listener, 0, sizeof(listener));
   listener.fileOpen = apx_eventRecorderSrvRmfMgr_onFileOpen;
}

static void apx_eventRecorderSrvRmfMgr_onFileOpen(void *arg, apx_fileManager_t *fileManager, const apx_file_t *file)
{
   apx_eventRecorderSrvRmfMgr_t *self = (apx_eventRecorderSrvRmfMgr_t*) arg;
   if ( (self != 0) && (fileManager != 0) && (file != 0))
   {
      if (file->fileInfo.address == APX_EVENT_LOG_FILE_ADDRESS)
      {
         printf("log file opened\n");
      }
   }
}
