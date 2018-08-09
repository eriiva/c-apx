/*****************************************************************************
* \file      apx_clientNodeManager.h
* \author    Conny Gustafsson
* \date      2018-08-09
* \brief     client node manager
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
#ifndef APX_CLIENT_NODE_MANAGER
#define APX_CLIENT_NODE_MANAGER

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "adt_hash.h"
#include "adt_list.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "osmacro.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_fileManager_tag;
struct apx_file_tag;
struct apx_router_tag;
struct apx_nodeData_tag;

typedef struct apx_clientNodeManager_tag
{
   adt_hash_t localNodeDataMap; //hash containing weak references to apx_nodeData_t for locally connected nodes. only used in client mode
   adt_list_t fileManagerList;
   MUTEX_T mutex; //locking mechanism
}apx_clientNodeManager_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_clientNodeManager_create(apx_clientNodeManager_t *self);
void apx_clientNodeManager_destroy(apx_clientNodeManager_t *self);
apx_clientNodeManager_t *apx_clientNodeManager_new(void);
void apx_clientNodeManager_delete(apx_clientNodeManager_t *self);
void apx_clientNodeManager_attachLocalNode(apx_clientNodeManager_t *self,  struct apx_nodeData_tag *nodeData);
void apx_clientNodeManager_attachFileManager(apx_clientNodeManager_t *self, struct apx_fileManager_tag *fileManager);
void apx_clientNodeManager_detachFileManager(apx_clientNodeManager_t *self, struct apx_fileManager_tag *fileManager);
struct apx_nodeData_tag *apx_clientNodeManager_findLocalNode(apx_clientNodeManager_t *self, const char *name);


#endif //APX_CLIENT_NODE_MANAGER
