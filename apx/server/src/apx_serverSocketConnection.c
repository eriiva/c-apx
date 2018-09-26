/*****************************************************************************
* \file      apx_srvSocketConnection.c
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
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
//#include <stdio.h>
#ifdef _MSC_VER
# ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# endif
#include <Windows.h>
#endif
#ifdef UNIT_TEST
#include "testsocket.h"
#else
#include "msocket.h"
#endif
#include "apx_serverSocketConnection.h"
#include "apx_logging.h"
#include "apx_transmitHandler.h"
#include "apx_fileManager.h"
#include "headerutil.h"
#include "bstr.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
#define SOCKET_TYPE testsocket_t
#define SOCKET_DELETE testsocket_delete
#define SOCKET_START_IO(x)
#define SOCKET_SET_HANDLER testsocket_setServerHandler
#else
#define SOCKET_DELETE msocket_delete
#define SOCKET_TYPE msocket_t
#define SOCKET_START_IO(x) msocket_start_io(x)
#define SOCKET_SET_HANDLER msocket_sethandler
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_serverSocketConnection_create(apx_serverSocketConnection_t *self, uint32_t connectionId, SOCKET_TYPE *socketObject, struct apx_server_tag *server)
{
   int8_t result = 0;
   result = apx_serverBaseConnection_create(&self->base, connectionId, server, apx_serverSocketConnection_vdestroy);
   if (result != 0)
   {
      return result;
   }
   self->socketObject = socketObject;
   return 0;
}

void apx_serverSocketConnection_destroy(apx_serverSocketConnection_t *self)
{

}

void apx_serverSocketConnection_vdestroy(void *arg)
{
   apx_serverSocketConnection_destroy((apx_serverSocketConnection_t*) arg);
}

apx_serverSocketConnection_t *apx_serverSocketConnection_new(uint32_t connectionId, SOCKET_TYPE *socketObject, struct apx_server_tag *server)
{
   apx_serverSocketConnection_t *self = (apx_serverSocketConnection_t*) malloc(sizeof(apx_serverSocketConnection_t));
   if (self != 0)
   {
      int8_t result = apx_serverSocketConnection_create(self, connectionId, socketObject, server);
      if (result != 0)
      {
         free(self);
         self = (apx_serverSocketConnection_t*) 0;
      }
   }
   return self;
}

void apx_serverSocketConnection_delete(apx_serverSocketConnection_t *self)
{

}

void apx_serverSocketConnection_vdelete(void *arg)
{

}

void apx_serverSocketConnection_start(apx_serverSocketConnection_t *self)
{
   if (self != 0)
   {
      SOCKET_START_IO(self->socketObject);
   }

}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


