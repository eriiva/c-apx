//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <malloc.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include "apx_portDataBuffer.h"
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
int8_t apx_portDataBuffer_create(apx_portDataBuffer_t *self, uint8_t *buf, uint32_t len)
{
   if ( (self != 0) && (len < ((uint32_t)MAX_PORT_DATA_LEN)) )
   {
      self->len=len;
      if ( (buf == 0) && (len>0) )
      {
         self->buf= (uint8_t*) malloc(len);
         if(self->buf==0)
         {
            return -1;
         }
         self->isAllocated = 1u;
      }
      else
      {
         self->buf=buf;
         self->isAllocated = 0u;
      }
      return 0;
   }
   return -1;
}

void apx_portDataBuffer_destroy(apx_portDataBuffer_t *self)
{
   if( (self !=0) && (self->isAllocated != 0) && (self->buf != 0) )
   {
      free(self->buf);
      self->buf = 0;
   }
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////