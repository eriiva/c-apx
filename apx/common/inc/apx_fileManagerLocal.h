/*****************************************************************************
* \file      apx_fileManagerLocal.h
* \author    Conny Gustafsson
* \date      2018-08-02
* \brief     APX Filemanager local representation
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
#ifndef APX_FILE_MANAGER_SENDER_H
#define APX_FILE_MANAGER_SENDER_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_fileManagerShared.h"
#include "apx_transmitHandler.h"


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_fileManagerSender_tag
{
   apx_transmitHandler_t transmitHandler;
   apx_fileManagerData_t sharedData; //weak pointer
}apx_fileManagerSender_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_fileManagerSender_create(apx_fileManagerSender_t *self, apx_fileManagerData_t *sharedData);
void apx_fileManagerSender_destroy(apx_fileManagerSender_t *self);
void apx_fileManagerSender_start(apx_fileManagerSender_t *self);
void apx_fileManagerSender_stop(apx_fileManagerSender_t *self);


#endif //APX_FILE_MANAGER_SENDER_H
