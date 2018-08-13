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
struct apx_file_tag;
struct rmf_fileInfo_tag;
struct apx_fileManager_tag;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//This is the API the eventListener can implement
typedef struct apx_eventListener_tag
{
   void *arg;
   void (*fileManagerStop)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*headerReceived)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*fileCreate)(void *arg, struct apx_fileManager_tag *fileManager, struct apx_file_tag *file);
   void (*fileRevoke)(void *arg, struct apx_fileManager_tag *fileManager, struct apx_file_tag *file);
   void (*fileOpen)(void *arg, struct apx_fileManager_tag *fileManager, const struct apx_file_tag *file);
   void (*fileClose)(void *arg, struct apx_fileManager_tag *fileManager, const struct apx_file_tag *file);
   void (*fileWrite)(void *arg, struct apx_fileManager_tag *fileManager, const struct apx_file_tag *file, uint32_t offset, int32_t length);
   void (*sendFileInfo)(void *arg, struct apx_fileManager_tag *fleManager, const struct rmf_fileInfo_tag *fileInfo);
} apx_eventListener_t;

//This is a base class for globally registered event listeners
typedef struct apx_eventListenerBase_tag
{
   void (*connected)(void *arg, struct apx_fileManager_tag *fileManager);
   void (*disconnected)(void *arg, struct apx_fileManager_tag *fileManager);
} apx_eventListenerBase_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_eventListener_t *apx_eventListener_clone(apx_eventListener_t *other);
void apx_eventListener_delete(apx_eventListener_t *self);
void apx_eventListener_vdelete(void *arg);

#endif //APX_EVENT_LISTENER_H
