/*****************************************************************************
* \file      apx_eventRecorderCltRmf.c
* \author    Conny Gustafsson
* \date      2018-08-13
* \brief     APX event recorder for clients running on the RemoteFile protocol
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
#include "apx_eventRecorderClientRmf.h"
#include "apx_fileManager.h"
#include "apx_eventListener.h"
#include "apx_eventFile.h"
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_eventRecorderClientRmf_connected(void *arg, apx_fileManager_t *fileManager);
static void apx_eventRecorderClientRmf_disconnected(void *arg, apx_fileManager_t *fileManager);
static void apx_eventRecorderClientRmf_onCreateFile(void *arg, apx_fileManager_t *fileManager, struct apx_file_tag *file);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderClientRmf_create(apx_eventRecorderClientRmf_t *self)
{
   if (self != 0)
   {
      self->base.connected = apx_eventRecorderClientRmf_connected;
      self->base.disconnected = apx_eventRecorderClientRmf_disconnected;
      self->fileManager = 0;
   }
}

void apx_eventRecorderClientRmf_destroy(apx_eventRecorderClientRmf_t *self)
{

}

apx_eventRecorderClientRmf_t *apx_eventRecorderClientRmf_new(void)
{
   apx_eventRecorderClientRmf_t *self = (apx_eventRecorderClientRmf_t*) malloc(sizeof(apx_eventRecorderClientRmf_t));
   if(self != 0)
   {
      apx_eventRecorderClientRmf_create(self);
   }
   return self;
}

void apx_eventRecorderClientRmf_delete(apx_eventRecorderClientRmf_t *self)
{
   if(self != 0)
   {
      apx_eventRecorderClientRmf_destroy(self);
      free(self);
   }
}

void apx_eventRecorderClientRmf_vdelete(void *arg)
{
   apx_eventRecorderClientRmf_delete((apx_eventRecorderClientRmf_t*) arg);
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void apx_eventRecorderClientRmf_connected(void *arg, apx_fileManager_t *fileManager)
{
   apx_fileManagerEventListener_t listener;
   memset(&listener, 0, sizeof(listener));
   listener.fileCreate = apx_eventRecorderClientRmf_onCreateFile;
   apx_fileManager_registerEventListener(fileManager, &listener);
}

static void apx_eventRecorderClientRmf_disconnected(void *arg, apx_fileManager_t *fileManager)
{
   printf("apx_eventRecorderClientRmf_disconnected\n");
}

static void apx_eventRecorderClientRmf_onCreateFile(void *arg, apx_fileManager_t *fileManager, struct apx_file_tag *file)
{
   if (file != 0)
   {
      if ( (file->isRemoteFile == true) && (strcmp(file->fileInfo.name, APX_EVENT_LOG_FILE_NAME)==0))
      {
         apx_fileManager_openRemoteFile(fileManager, file->fileInfo.address);
      }
   }
}
