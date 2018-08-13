/*****************************************************************************
* \file      apx_eventRecorderClientRmf.h
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
#ifndef APX_EVENT_RECORDER_CLIENT_RMF_H
#define APX_EVENT_RECORDER_CLIENT_RMF_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_eventListener.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_fileManager_tag;

typedef struct apx_eventRecorderClientRmf_tag
{
   apx_eventListenerBase_t base;
   struct apx_fileManager_tag *fileManager;
} apx_eventRecorderClientRmf_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderClientRmf_create(apx_eventRecorderClientRmf_t *self);
void apx_eventRecorderClientRmf_destroy(apx_eventRecorderClientRmf_t *self);
apx_eventRecorderClientRmf_t *apx_eventRecorderClientRmf_new(void);
void apx_eventRecorderClientRmf_delete(apx_eventRecorderClientRmf_t *self);
void apx_eventRecorderClientRmf_vdelete(void *arg);

#endif //APX_EVENT_RECORDER_CLIENT_RMF_H
