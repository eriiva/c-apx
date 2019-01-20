/*****************************************************************************
* \file      apx_clientTestConnection.c
* \author    Conny Gustafsson
* \date      2019-01-09
* \brief     Test connection for APX clients
*
* Copyright (c) 2019 Conny Gustafsson
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
#include <malloc.h>
#include <stdio.h> //debug only
#include "apx_clientTestConnection.h"
#include "apx_clientInternal.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_clientTestConnection_create(apx_clientTestConnection_t *self, struct apx_client_tag *client)
{
   if (self != 0)
   {
      apx_connectionBaseVTable_t vtable;
      apx_connectionBaseVTable_create(&vtable, apx_clientTestConnection_vdestroy, apx_clientTestConnection_vstart, apx_clientTestConnection_vclose);
      apx_error_t result = apx_clientConnectionBase_create(&self->base, client, &vtable);
      return result;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_clientTestConnection_destroy(apx_clientTestConnection_t *self)
{
   if (self != 0)
   {
      apx_clientConnectionBase_destroy(&self->base);
   }
}

void apx_clientTestConnection_vdestroy(void *arg)
{
   apx_clientTestConnection_destroy((apx_clientTestConnection_t*) arg);
}

apx_clientTestConnection_t *apx_clientTestConnection_new(struct apx_client_tag *client)
{
   apx_clientTestConnection_t *self = (apx_clientTestConnection_t*) malloc(sizeof(apx_clientTestConnection_t));
   if (self != 0)
   {
      apx_error_t result = apx_clientTestConnection_create(self, client);
      if (result != APX_NO_ERROR)
      {
         apx_setError(result);
         free(self);
         self = 0;
      }
   }
   else
   {
      apx_setError(APX_MEM_ERROR);
   }
   return self;
}

void apx_clientTestConnection_delete(apx_clientTestConnection_t *self)
{
   if (self != 0)
   {
      apx_clientTestConnection_destroy(self);
      free(self);
   }
}

void apx_clientTestConnection_start(apx_clientTestConnection_t *self)
{
   apx_clientConnectionBase_start(&self->base);
}

void apx_clientTestConnection_vstart(void *arg)
{
   apx_clientTestConnection_start((apx_clientTestConnection_t*) arg);
}

void apx_clientTestConnection_close(apx_clientTestConnection_t *self)
{

}

void apx_clientTestConnection_vclose(void *arg)
{
   apx_clientTestConnection_close((apx_clientTestConnection_t*) arg);
}


/**
 * Remote side has created a new remote file
 */
void apx_clientTestConnection_createRemoteFile(apx_clientTestConnection_t *self, const rmf_fileInfo_t *fileInfo)
{
   if ( (self != 0) && (fileInfo != 0) )
   {
      apx_fileManager_onRemoteCmdFileInfo(&self->base.base.fileManager, fileInfo);
   }
}

/**
 * Writes raw data to a remote address
 */
void apx_clientTestConnection_writeRemoteData(apx_clientTestConnection_t *self, uint32_t address, const uint8_t* dataBuf, uint32_t dataLen, bool more)
{
   if ( (self != 0) && (dataBuf != 0) )
   {
      apx_fileManager_onWriteRemoteData(&self->base.base.fileManager, address, dataBuf, dataLen, more);
   }
}

void apx_clientTestConnection_openRemoteFile(apx_clientTestConnection_t *self, uint32_t address)
{
   if (self != 0)
   {
      apx_fileManager_onRemoteCmdFileOpen(&self->base.base.fileManager, address);
   }
}

void apx_clientTestConnection_runEventLoop(apx_clientTestConnection_t *self)
{
   if (self != 0)
   {
#ifdef UNIT_TEST
      apx_connectionBase_runAll(&self->base.base);
#endif
   }
}

void apx_clientTestConnection_connect(apx_clientTestConnection_t *self)
{
   if (self != 0)
   {
      apx_clientConnectionBase_onConnected(&self->base);
   }
}

void apx_clientTestConnection_disconnect(apx_clientTestConnection_t *self)
{
   if (self != 0)
   {
      apx_clientConnectionBase_onDisconnected(&self->base);
   }
}


//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

