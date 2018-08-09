/*****************************************************************************
* \file      apx_fileManager.h
* \author    Conny Gustafsson
* \date      2018-08-04
* \brief     New APX file manager
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
#ifndef APX_FILE_MANAGER_H
#define APX_FILE_MANAGER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_fileManagerDefs.h"
#include "apx_fileManagerShared.h"
#include "apx_fileManagerRemote.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_fileManager_tag
{
   apx_fileManagerShared_t shared;
   apx_fileManagerRemote_t remote;
   adt_list_t eventListeners; //contains strong references to apx_eventListener_t
   uint8_t mode;
}apx_fileManager_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_fileManager_create(apx_fileManager_t *self, uint8_t mode, uint32_t connectionId);
void apx_fileManager_destroy(apx_fileManager_t *self);

void* apx_fileManager_registerEventListener(apx_fileManager_t *self, struct apx_eventListener_tag* listener);
void apx_fileManager_unregisterEventListener(apx_fileManager_t *self, void *handle);
int32_t apx_fileManager_getNumEventListeners(apx_fileManager_t *self);

void apx_fileManager_start(apx_fileManager_t *self);
void apx_fileManager_onHeaderAccepted(apx_fileManager_t *self);
int32_t apx_fileManager_parseMessage(apx_fileManager_t *self, const uint8_t *msgBuf, int32_t msgLen);
uint32_t fileManager_getID(apx_fileManager_t *self);


#endif //APX_FILE_MANAGER_H
