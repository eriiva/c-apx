/*****************************************************************************
* \file      apx_eventRecorderSrvRmfMgr.h
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
#ifndef APX_EVENT_RECORDER_SRV_RMF_MGR_H
#define APX_EVENT_RECORDER_SRV_RMF_MGR_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_eventListener.h"
#include "adt_list.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#ifndef APX_EVENT_RECORDER_RMF_DEFAULT_UPDATE_TIME
#define APX_EVENT_RECORDER_RMF_DEFAULT_UPDATE_TIME 10  //resolution: 100 milliseconds per bit
#endif


//forward declarations


typedef struct apx_eventRecorderSrvRmfMgr_tag
{
   apx_connectionEventListener_t base;
   adt_list_t instanceList; //strong references to apx_eventRecorderSrvRmf_t
   int32_t minimumUpdateTime;
} apx_eventRecorderSrvRmfMgr_t;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_eventRecorderSrvRmfMgr_create(apx_eventRecorderSrvRmfMgr_t *self, int32_t minimumUpdateTime);
void apx_eventRecorderSrvRmfMgr_destroy(apx_eventRecorderSrvRmfMgr_t *self);
apx_eventRecorderSrvRmfMgr_t *apx_eventRecorderSrvRmfMgr_new(int32_t minimumUpdateTime);
void apx_eventRecorderSrvRmfMgr_delete(apx_eventRecorderSrvRmfMgr_t *self);

#endif //APX_EVENT_RECORDER_SRV_RMF_MGR_H
