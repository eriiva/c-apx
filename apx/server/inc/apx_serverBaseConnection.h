/*****************************************************************************
* \file      apx_srvBaseConnection.h
* \author    Conny Gustafsson
* \date      2018-09-26
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
#ifndef APX_SERVER_BASE_CONNECTION_H
#define APX_SERVER_BASE_CONNECTION_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"
#include "apx_error.h"
#include "apx_fileManager.h"
#include "apx_nodeDataManager.h"
#include "apx_eventLoop.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
struct apx_server_tag;

typedef struct apx_serverBaseConnection_tag
{
   apx_fileManager_t fileManager;
   apx_nodeDataManager_t nodeDataManager;
   apx_eventLoop_t eventLoop;
   uint32_t connectionId;
   struct apx_server_tag *server;
   bool isGreetingParsed;
   uint8_t numHeaderLen; //0, 2 or 4
   struct
   {
      void (*destructor)(void *arg);
   }vtable;
}apx_serverBaseConnection_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_serverBaseConnection_create(apx_serverBaseConnection_t *self, uint32_t connectionId, struct apx_server_tag *server, void (*destructor)(void *arg));
void apx_serverBaseConnection_destroy(apx_serverBaseConnection_t *self);
//apx_serverBaseConnection_t *apx_serverBaseConnection_new(void);
void apx_serverBaseConnection_delete(apx_serverBaseConnection_t *self);
void apx_serverBaseConnection_vdelete(void *arg);

apx_fileManager_t *apx_serverBaseConnection_getFileManager(apx_serverBaseConnection_t *self);
int8_t apx_serverBaseConnection_dataReceived(apx_serverBaseConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
void apx_serverBaseConnection_start(apx_serverBaseConnection_t *self);

#ifdef UNIT_TEST
void apx_serverBaseConnection_run(apx_serverBaseConnection_t *self);
#endif

#endif //APX_SERVER_BASE_CONNECTION_H
