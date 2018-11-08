/*****************************************************************************
* \file      apx_eventListener.h
* \author    Conny Gustafsson
* \date      2018-05-01
* \brief     Interface for internal event listerner
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
#ifndef APX_EVENT_LISTENER_H
#define APX_EVENT_LISTENER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

//forward declarations
struct apx_file2_tag;
struct rmf_fileInfo_tag;
struct apx_fileManager_tag;
struct apx_nodeData_tag;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

typedef struct apx_fileManagerEventListener_tag
{
   void *arg;
   void (*fileManagerPostStop)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*headerComplete)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*fileCreate)(void *arg, struct apx_fileManager_tag *fileManager, struct apx_file2_tag *file);
   void (*fileRevoke)(void *arg, struct apx_fileManager_tag *fileManager, struct apx_file2_tag *file);
   void (*fileOpen)(void *arg, struct apx_fileManager_tag *fileManager, const struct apx_file2_tag *file);
   void (*fileClose)(void *arg, struct apx_fileManager_tag *fileManager, const struct apx_file2_tag *file);
} apx_fileManagerEventListener_t;

//This is a base class and must be placed as the first element in a struct definition
typedef struct apx_connectionEventListener_tag
{
   void (*connected)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*disconnected)(void *arg, struct apx_fileManager_tag *fileManager);
} apx_connectionEventListener_t;

typedef struct apx_nodeDataEventListener_tag
{
   void *arg; //user argument
   void (*definitionDataWritten)(void *arg, struct apx_nodeData_tag *nodeData, uint32_t offset, uint32_t len);
   void (*inPortDataWritten)(void *arg, struct apx_nodeData_tag *nodeData, uint32_t offset, uint32_t len);
   void (*outPortDataWritten)(void *arg, struct apx_nodeData_tag *nodeData, uint32_t offset, uint32_t len);
   void (*nodeComplete)(void *arg, struct apx_nodeData_tag *nodeData);
}apx_nodeDataEventListener_t;

typedef void (apx_eventListener_connectedFunc_t)(void *arg, struct apx_fileManager_tag *fileManager);
typedef void (apx_eventListener_disconnectedFunc_t)(void *arg, struct apx_fileManager_tag *fileManager);
typedef void (apx_eventListener_fileManagerStartFunc_t)(void *arg, struct apx_fileManager_tag *fileManager);
typedef void (apx_eventListener_fileManagerStopFunc_t)(void *arg, struct apx_fileManager_tag *fileManager);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_fileManagerEventListener_t *apx_fileManagerEventListener_clone(apx_fileManagerEventListener_t *other);
void apx_fileManagerEventListener_delete(apx_fileManagerEventListener_t *self);
void apx_fileManagerEventListener_vdelete(void *arg);

#endif //APX_EVENT_LISTENER_H
