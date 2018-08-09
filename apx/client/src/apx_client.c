//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "apx_client.h"
#include "apx_clientConnection.h"
#include "apx_clientNodeManager.h"
#include "msocket.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif



//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_client_create(apx_client_t *self)
{
   if( self != 0 )
   {
      self->nodeManager = apx_clientNodeManager_new();
      if (self->nodeManager == 0)
      {
         return -1;
      }
      self->connection = apx_clientConnection_new(self);
      if (self->connection == 0)
      {
         apx_clientNodeManager_delete(self->nodeManager);
         return -1;
      }
      return 0;
   }
   errno=EINVAL;
   return -1;
}

void apx_client_destroy(apx_client_t *self)
{
   if (self != 0)
   {
      apx_clientConnection_delete(self->connection);
      apx_clientNodeManager_delete(self->nodeManager);
   }
}

apx_client_t *apx_client_new(void)
{
   apx_client_t *self = (apx_client_t*) malloc(sizeof(apx_client_t));
   if(self != 0)
   {
      int8_t result = apx_client_create(self);
      if (result != 0)
      {
         free(self);
         self=0;
      }
   }
   else
   {
      errno = ENOMEM;
   }
   return self;
}

void apx_client_delete(apx_client_t *self)
{
   if (self != 0)
   {
      apx_client_destroy(self);
      free(self);
   }
}

void apx_client_vdelete(void *arg)
{
   apx_client_delete((apx_client_t*) arg);
}

#ifdef UNIT_TEST
int8_t apx_client_connect(apx_client_t *self, struct testsocket_tag *socketObject)
{
   if (self != 0)
   {
      return apx_clientConnection_connect(self->connection, socketObject);
   }
   return -1;
}
#else
int8_t apx_client_connectTcp(apx_client_t *self, const char *address, uint16_t port)
{
   if (self != 0)
   {
      return apx_clientConnection_connectTcp(self->connection, address, port);
   }
}
#endif


/**
 * attached the nodeData to the local nodeManager in the client
 */
void apx_client_attachLocalNode(apx_client_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      apx_clientNodeManager_attachLocalNode(self->nodeManager, nodeData);
   }
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


