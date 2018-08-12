/*****************************************************************************
* \file      apx_eventRecorderSrvTxt.c
* \author    Conny Gustafsson
* \date      2018-08-07
* \brief     Description
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
#include <stdio.h>
#include "adt_str.h"
#include "apx_eventRecorderSrvTxt.h"
#include "apx_eventListener.h"
#include "apx_fileManager.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif
//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#define STRDUP _strdup
#else
#define STRDUP strdup
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_eventRecorderSrvTxt_registerAsListener(apx_eventRecorderSrvTxt_t *self, apx_fileManager_t *fileManager);
static bool apx_eventRecorderSrvTxt_isLogFileOpen(apx_eventRecorderSrvTxt_t *self);
static void apx_eventRecorderSrvTxt_onConnected(void *arg, struct apx_fileManager_tag *fileManager);
static void apx_eventRecorderSrvTxt_onDisconnected(void *arg, struct apx_fileManager_tag *fileManager);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderSrvTxt_create(apx_eventRecorderSrvTxt_t *self)
{
   if (self != 0)
   {
      self->base.newConnection = apx_eventRecorderSrvTxt_onConnected;
      self->fileName = 0;
      self->fp = 0;
   }
}

void apx_eventRecorderSrvTxt_destroy(apx_eventRecorderSrvTxt_t *self)
{
   if (self != 0)
   {
      apx_eventRecorderSrvTxt_close(self);
      if (self->fileName !=0)
      {
         free(self->fileName);
         self->fileName = 0;
      }
   }
}

apx_eventRecorderSrvTxt_t *apx_eventRecorderSrvTxt_new(void)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t*) malloc(sizeof(apx_eventRecorderSrvTxt_t));
   if(self != 0)
   {
      apx_eventRecorderSrvTxt_create(self);
   }
   return self;
}

void apx_eventRecorderSrvTxt_delete(apx_eventRecorderSrvTxt_t *self)
{
   if(self != 0)
   {
      apx_eventRecorderSrvTxt_destroy(self);
      free(self);
   }
}

void apx_eventRecorderSrvTxt_open(apx_eventRecorderSrvTxt_t *self, const char *fileName)
{
   if ( (self != 0) && (fileName != 0) )
   {
      self->fileName = STRDUP(fileName);
      self->fp = fopen(fileName,"w");
      if (self->fp == 0)
      {
         adt_str_t *msg = adt_str_new();
         adt_str_append_cstr(msg, "[APX] Unable to open log file: ");
         adt_str_append_cstr(msg, fileName);
      }
      else
      {
         fprintf(self->fp, "header\n");
      }
   }
}

void apx_eventRecorderSrvTxt_close(apx_eventRecorderSrvTxt_t *self)
{
   if ( (self != 0) && (self->fp != 0) )
   {
      fclose(self->fp);
      self->fp = 0;
   }
}


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void apx_eventRecorderSrvTxt_registerAsListener(apx_eventRecorderSrvTxt_t *self, struct apx_fileManager_tag *fileManager)
{
   apx_eventListener_t listener;
   memset(&listener, 0, sizeof(listener));
   listener.arg = self;
   listener.fileManagerStop = apx_eventRecorderSrvTxt_onDisconnected;
   apx_fileManager_registerEventListener(fileManager, &listener);
}

static bool apx_eventRecorderSrvTxt_isLogFileOpen(apx_eventRecorderSrvTxt_t *self)
{
   if ( (self != 0) && (self->fp != 0))
   {
      return true;
   }
   return false;
}

static void apx_eventRecorderSrvTxt_onConnected(void *arg, struct apx_fileManager_tag *fileManager)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t *) arg;
   if ( (self != 0) && (fileManager != 0) )
   {
      if (apx_eventRecorderSrvTxt_isLogFileOpen(self) == true)
      {
         fprintf(self->fp, "[%u] Client connected\n", fileManager_getID(fileManager));
      }
      apx_eventRecorderSrvTxt_registerAsListener(self, fileManager);
   }
}

static void apx_eventRecorderSrvTxt_onDisconnected(void *arg, struct apx_fileManager_tag *fileManager)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t *) arg;
   if ( (self != 0) && (fileManager != 0) && (self->fp != 0) )
   {
      fprintf(self->fp, "[%u] Client disconnected\n", fileManager_getID(fileManager));
   }
}


