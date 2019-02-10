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
#include "apx_serverConnectionBase.h"
#include "apx_portConnectionTable.h"
#include "apx_server.h"

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
static void apx_eventRecorderSrvTxt_registerNodeDataListener(apx_eventRecorderSrvTxt_t *self, apx_serverConnectionBase_t *connection);
static bool apx_eventRecorderSrvTxt_isLogFileOpen(apx_eventRecorderSrvTxt_t *self);
static void apx_eventRecorderSrvTxt_onConnected(void *arg, apx_serverConnectionBase_t *connection);
static void apx_eventRecorderSrvTxt_onDisconnected(void *arg, apx_serverConnectionBase_t *connection);
static void apx_eventRecorderSrvTxt_providePortsConnected(void *arg, apx_nodeData_t *nodeData, apx_portConnectionTable_t *connectionTable);
static void apx_eventRecorderSrvTxt_providePortsDisconnected(void *arg, apx_nodeData_t *nodeData, apx_portConnectionTable_t *connectionTable);

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

void apx_eventRecorderSrvTxt_register(apx_eventRecorderSrvTxt_t *self, struct apx_server_tag *server)
{
   apx_serverEventListener_t eventListener;
   memset(&eventListener, 0, sizeof(apx_serverEventListener_t));
   eventListener.arg = (void*) self;
   eventListener.serverConnected = apx_eventRecorderSrvTxt_onConnected;
   eventListener.serverDisconnected = apx_eventRecorderSrvTxt_onDisconnected;
   apx_server_registerEventListener(server, &eventListener);
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
static void apx_eventRecorderSrvTxt_registerNodeDataListener(apx_eventRecorderSrvTxt_t *self, apx_serverConnectionBase_t *connection)
{
   apx_nodeDataEventListener_t listener;
   memset(&listener, 0, sizeof(listener));
   listener.arg = (void*) self;
   listener.providePortsConnected = apx_eventRecorderSrvTxt_providePortsConnected;
   listener.providePortsDisconnected = apx_eventRecorderSrvTxt_providePortsDisconnected;
   apx_serverConnectionBase_registerNodeDataEventListener(connection, &listener);
}

static bool apx_eventRecorderSrvTxt_isLogFileOpen(apx_eventRecorderSrvTxt_t *self)
{
   if ( (self != 0) && (self->fp != 0))
   {
      return true;
   }
   return false;
}

static void apx_eventRecorderSrvTxt_onConnected(void *arg, apx_serverConnectionBase_t *connection)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t *) arg;
   if ( (self != 0) && (connection != 0) )
   {
      if (apx_eventRecorderSrvTxt_isLogFileOpen(self) == true)
      {
         fprintf(self->fp, "[%u] Client connected\n", apx_serverConnectionBase_getConnectionId(connection));
      }
      apx_eventRecorderSrvTxt_registerNodeDataListener(self, connection);
   }
}

static void apx_eventRecorderSrvTxt_onDisconnected(void *arg, apx_serverConnectionBase_t *connection)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t *) arg;
   if ( (self != 0) && (connection != 0) && (self->fp != 0) )
   {
      fprintf(self->fp, "[%u] Client disconnected\n", apx_serverConnectionBase_getConnectionId(connection));
   }
}

static void apx_eventRecorderSrvTxt_providePortsConnected(void *arg, apx_nodeData_t *nodeData, apx_portConnectionTable_t *connectionTable)
{
   apx_eventRecorderSrvTxt_t *self = (apx_eventRecorderSrvTxt_t *) arg;
   if ( (self != 0) && (connectionTable != 0) && (self->fp != 0) )
   {
      int32_t portId;
      for (portId=0; portId<connectionTable->numPorts; portId++)
      {
         apx_portConnectionEntry_t *entry = apx_portConnectionTable_getEntry(connectionTable, portId);
         fprintf(self->fp, "PC %s.%d: %d\n", apx_nodeData_getName(nodeData), (int) portId, (int) apx_portConnectionEntry_count(entry));
      }
   }
}

static void apx_eventRecorderSrvTxt_providePortsDisconnected(void *arg, apx_nodeData_t *nodeData, apx_portConnectionTable_t *connectionTable)
{

}

