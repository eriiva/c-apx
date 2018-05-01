/*****************************************************************************
* \file:    apx_clientEventPlayer.h
* \author:  Conny Gustafsson
* \date:    2018-05-01
* \brief:   An APX client player reads a log file and sends stored events to server as a series of messages.
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

#ifndef APX_CLIENT_EVENT_PLAYER_H
#define APX_CLIENT_EVENT_PLAYER_H
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "rmf.h"
#include "apx_file.h"
#include "apx_event.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef struct apx_clientEventPlayer_tag
{
   apx_file_t *file;
}apx_clientEventPlayer_t;

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_clientEventPlayer_create(apx_clientEventPlayer_t *self);
void apx_clientEventPlayer_destroy(apx_clientEventPlayer_t *self);
apx_clientEventPlayer_t *apx_clientEventPlayer_new(void);
void apx_clientEventPlayer_delete(apx_clientEventPlayer_t *self);

#endif //APX_CLIENT_EVENT_PLAYER_H
