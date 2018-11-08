/*****************************************************************************
* \file      apx_event.h
* \author    Conny Gustafsson
* \date      2018-10-15
* \brief     Maps all APX event listeners event data into a common data structure
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
#ifndef APX_EVENT_H
#define APX_EVENT_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_event_tag
{
   uint16_t evType;
   uint16_t evFlags;
   void *handler;    //function pointer
   void *evData1;    //generic void* pointer value
   void *evData2;    //generic void* pointer value
   void *evData3;    //generic void* pointer value
   void *evData4;    //generic void* pointer value
   uint32_t evData5; //generic uint32 value
   uint32_t evData6; //generic uint32 value
} apx_event_t;

#define APX_EVENT_SIZE sizeof(apx_event_t)

#define APX_EVENT_FLAG_INTERNAL            0x01
#define APX_EVENT_FLAG_DEFINITION_FILE     0x02
#define APX_EVENT_FLAG_IN_DATA_FILE        0x04
#define APX_EVENT_FLAG_OUT_DATA_FILE       0x08
#define APX_EVENT_FLAG_FILE_COMPLETE       0x10

//APX connection events
#define APX_EVENT_APX_CONNECTED            0 //evData1: *arg, evData2:*fileManager
#define APX_EVENT_APX_DISCONNECTED         1 //evData1:*arg, evData2:*fileManager

//Remote file manager events
#define APX_EVENT_RMF_MANAGER_PRE_START    2 //evData1:*arg, evData2:*fileManager
#define APX_EVENT_RMF_MANAGER_POST_STOP    3 //evData1:*arg, evData2:*fileManager
#define APX_EVENT_RMF_HEADER_COMPLETE      4 //evData1:*arg, evData2:*fileManager
#define APX_EVENT_RMF_FILE_CREATED         5 //evData1:*arg, evData2:*fileManager, evData3:*file
#define APX_EVENT_RMF_FILE_REVOKED         6 //evData1:*arg, evData2:*fileManager, evData3:*file
#define APX_EVENT_RMF_FILE_OPENED          7 //evData1:*arg, evData2:*fileManager, evData3:*file
#define APX_EVENT_RMF_FILE_CLOSED          8 //evData1:*arg, evData2:*fileManager, evData3:*file

//Remote file data events
#define APX_EVENT_RMF_FILE_WRITE           9 //evData1:*arg, evData2:*fileManager, evData3:*file, evData4: *u8Data, evData5: offset, evData6: len, evData7: evFlags

//APX node events
/*
#define APX_EVENT_NODE_DEFINITION_WRITE    8  //evData1:*arg, evData2:*nodeData, evData4: offset, evData5: len
#define APX_EVENT_NODE_INDATA_WRITE        9  //evData1:*arg, evData2:*nodeData, evData4: offset, evData5: len
#define APX_EVENT_NODE_OUTATA_WRITE        10 //evData1:*arg, evData2:*nodeData, evData4: offset, evData5: len
#define APX_EVENT_NODE_COMPLETE            11 //evData1:*arg, evData2:*nodeData
*/

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////


#endif //APX_EVENT_H
