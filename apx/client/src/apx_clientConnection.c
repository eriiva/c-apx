//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "apx_clientConnection.h"
#include "apx_client.h"
#include "apx_transmitHandler.h"
#include "headerutil.h"
#include "apx_fileManager.h"
#include "apx_nodeManager.h"
#ifdef UNIT_TEST
#include "testsocket.h"
#else
#include "msocket.h"
#endif
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif



//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define MAX_HEADER_LEN 128
#define SEND_BUFFER_GROW_SIZE 4096 //4KB

#ifdef UNIT_TEST
#define SOCKET_TYPE struct testsocket_tag
#define APX_SOCKET_CLOSE(X)
#define SOCKET_DELETE testsocket_delete
#define SOCKET_SEND testsocket_clientSend
#define SOCKET_SET_HANDLER testsocket_setClientHandler
#else
#define SOCKET_TYPE struct msocket_t
#define APX_SOCKET_CLOSE msocket_close
#define SOCKET_DELETE msocket_delete
#define SOCKET_SEND msocket_send
#define SOCKET_SET_HANDLER msocket_sethandler
#endif


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_clientConnection_registerSocketHandler(apx_clientConnection_t *self, SOCKET_TYPE *socketObject);
static int8_t apx_clientConnection_data(void *arg, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
static void apx_clientConnection_disconnected(void *arg);
static void apx_clientConnection_connected(void *arg, const char *addr, uint16_t port);
static int8_t apx_clientConnection_parseMessage(apx_clientConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);
//primary transmitHandler interface
static uint8_t *apx_clientConnection_getSendBuffer(void *arg, int32_t msgLen);
static int32_t apx_clientConnection_send(void *arg, int32_t offset, int32_t msgLen);
//secondary transmitHandler interface
static uint8_t* apx_clientConnection_getSendBufferRaw(void *arg, int32_t dataLen);
static int32_t apx_clientConnection_sendRaw(void *arg, int32_t dataLen);

static void apx_clientConnection_sendGreeting(apx_clientConnection_t *self);
static void apx_clientConnection_start(apx_clientConnection_t *self);
static void apx_clientConnection_stop(apx_clientConnection_t *self);
static int8_t apx_clientConnection_dataReceived(apx_clientConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen);


//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_clientConnection_create(apx_clientConnection_t *self, struct apx_client_tag *client)
{
   if( (self != 0) &&  (client != 0))
   {
      self->socketObject = (SOCKET_TYPE*) 0;
      self->isAcknowledgeSeen = false;
      self->client = client;
      self->maxMsgHeaderSize = (uint8_t) sizeof(uint32_t);
      self->isConnected = false;
      apx_fileManager_create(&self->fileManager, APX_FILEMANAGER_CLIENT_MODE, 0);
      adt_bytearray_create(&self->sendBuffer, SEND_BUFFER_GROW_SIZE);
      SPINLOCK_INIT(self->lock);
      return 0;
   }
   errno=EINVAL;
   return -1;
}

void apx_clientConnection_destroy(apx_clientConnection_t *self)
{
   if (self != 0)
   {
      if (self->socketObject != 0)
      {
         SOCKET_DELETE(self->socketObject);
      }
      apx_fileManager_destroy(&self->fileManager);
      adt_bytearray_destroy(&self->sendBuffer);
      SPINLOCK_DESTROY(self->lock);
   }
}

apx_clientConnection_t *apx_clientConnection_new(struct apx_client_tag *client)
{
   if ( client != 0 )
   {
      apx_clientConnection_t *self = (apx_clientConnection_t*) malloc(sizeof(apx_clientConnection_t));
      if(self != 0)
      {
         int8_t result = apx_clientConnection_create(self, client);
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
   return (apx_clientConnection_t*) 0;
}

void apx_clientConnection_delete(apx_clientConnection_t *self)
{
   if (self != 0)
   {
      apx_clientConnection_destroy(self);
      free(self);
   }
}

void apx_clientConnection_vdelete(void *arg)
{
   apx_clientConnection_delete((apx_clientConnection_t*) arg);
}

#ifdef UNIT_TEST
int8_t apx_clientConnection_connect(apx_clientConnection_t *self, SOCKET_TYPE *socketObject)
{
   if ( (self != 0) && (socketObject != 0) )
   {
      apx_clientConnection_registerSocketHandler(self, socketObject);
      self->socketObject = socketObject;
      return 0;
   }
   return -1;
}

#else
int8_t apx_clientConnection_connectTcp(apx_clientConnection_t *self, const char *address, uint16_t port)
{
   if (self != 0)
   {
      msocket_t *socketObject = msocket_new(AF_INET);
      if (socketObject != 0)
      {
         int8_t retval = 0;
         apx_clientConnection_registerSocketHandler(self, socketObject);
         retval = msocket_connect(socketObject, address, port);
         if (retval != 0)
         {
            fprintf(stderr, "[apx_client] msocket_connect failed with %d\n",retval);
            msocket_delete(socketObject);
         }
         self->socketObject = socketObject;
         return retval;
      }
      else
      {
         fprintf(stderr, "[apx_client] msocket_new returned NULL\n");
      }
   }
   return -1;
}
#endif


void apx_clientConnection_disconnect(apx_clientConnection_t *self)
{
   if ( (self != 0) && (self->isConnected == true) )
   {
      apx_clientConnection_stop(self);
      APX_SOCKET_CLOSE(self->socketObject);
      SOCKET_DELETE(self->socketObject);
      self->socketObject = 0;
      self->isConnected = false;
   }
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void apx_clientConnection_registerSocketHandler(apx_clientConnection_t *self, SOCKET_TYPE *socketObject)
{
   if (socketObject != 0)
   {
      msocket_handler_t handlerTable;
      memset(&handlerTable,0,sizeof(handlerTable));
      handlerTable.tcp_connected=apx_clientConnection_connected;
      handlerTable.tcp_data=apx_clientConnection_data;
      handlerTable.tcp_disconnected = apx_clientConnection_disconnected;
      SOCKET_SET_HANDLER(socketObject,&handlerTable, self);
   }
}

static int8_t apx_clientConnection_data(void *arg, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   return apx_clientConnection_dataReceived(self, dataBuf, dataLen, parseLen);
}

static void apx_clientConnection_disconnected(void *arg)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if (self != 0)
   {
      SPINLOCK_ENTER(self->lock);
      self->isConnected = false;
      SPINLOCK_LEAVE(self->lock);
      apx_clientConnection_stop(self);
      _apx_client_on_disconnect(self->client, &self->fileManager);
   }
}

static void apx_clientConnection_connected(void *arg,const char *addr,uint16_t port)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if (self != 0)
   {
      SPINLOCK_ENTER(self->lock);
      self->isConnected = true;
      SPINLOCK_LEAVE(self->lock);
      _apx_client_on_connect(self->client, &self->fileManager);
      apx_clientConnection_start(self);
   }
}

/**
 * activates the server connection and sends out the greeting
 */
void apx_clientConnection_start(apx_clientConnection_t *self)
{
      apx_transmitHandler_t transmitHandler;
      //register transmit handler with our fileManager
      transmitHandler.arg = self;
      transmitHandler.send = apx_clientConnection_send;
      transmitHandler.getSendAvail = 0;
      transmitHandler.getSendBuffer = apx_clientConnection_getSendBuffer;
      transmitHandler.getSendBufferRaw = apx_clientConnection_getSendBufferRaw;
      transmitHandler.sendRaw = apx_clientConnection_sendRaw;
      apx_fileManager_setTransmitHandler(&self->fileManager, &transmitHandler);
      //register connection with the server nodeManager
      apx_nodeManager_attachFileManager(self->client->nodeManager, &self->fileManager);
      apx_fileManager_start(&self->fileManager);
      apx_clientConnection_sendGreeting(self);
}

static void apx_clientConnection_stop(apx_clientConnection_t *self)
{
   if (self != 0)
   {
      apx_fileManager_stop(&self->fileManager);
   }
}

void apx_clientConnection_sendGreeting(apx_clientConnection_t *self)
{
   uint8_t *sendBuffer;
   uint32_t greetingLen;
   char greeting[RMF_GREETING_MAX_LEN];
   char *p = &greeting[0];
   strcpy(greeting, RMF_GREETING_START);
   p += strlen(greeting);
   p += sprintf(p, "%s32\n\n", RMF_NUMHEADER_FORMAT);
   greetingLen = (uint32_t) (p-greeting);
   sendBuffer = apx_clientConnection_getSendBuffer((void*) self, greetingLen);
   if (sendBuffer != 0)
   {
      memcpy(sendBuffer, greeting, greetingLen);
      apx_clientConnection_send((void*) self, 0, greetingLen);
   }
   else
   {
      fprintf(stderr, "Failed to acquire sendBuffer while trying to send greeting\n");
   }
}

/**
 * a message consists of a message length (first 1 or 4 bytes) packed as binary integer (big endian).
 * Then follows the message data followed by a new message length header etc.
 * Returns 0 on parse success, -1 on parse failure.
 */
static int8_t apx_clientConnection_parseMessage(apx_clientConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   uint32_t msgLen;
   const uint8_t *pBegin = dataBuf;
   const uint8_t *pResult;
   const uint8_t *pEnd = dataBuf+dataLen;
   const uint8_t *pNext = pBegin;
   pResult = headerutil_numDecode32(pNext, pEnd, &msgLen);
   if (pResult>pNext)
   {
      uint32_t headerLen = (uint32_t) (pResult-pNext);
      pNext = pResult;
      //TODO: implement sanity check for too long messages? (by checking value of msgLen here)
      if (pNext+msgLen<=pEnd)
      {
         if (parseLen != 0)
         {
            *parseLen=headerLen+msgLen;
         }         
         if (self->isAcknowledgeSeen == false)
         {
            if (msgLen == 8)
            {
               if ( (pNext[0] == 0xbf) &&
                    (pNext[1] == 0xff) &&
                    (pNext[2] == 0xfc) &&
                    (pNext[3] == 0x00) &&
                    (pNext[4] == 0x00) &&
                    (pNext[5] == 0x00) &&
                    (pNext[6] == 0x00) &&
                    (pNext[7] == 0x00) )
               {
                  self->isAcknowledgeSeen = true;
                  apx_fileManager_onHeaderAccepted(&self->fileManager);
               }
            }
         }
         else
         {
            apx_fileManager_parseMessage(&self->fileManager, pNext, msgLen);
         }
      }
      else
      {
         //we have to wait until entire message is in the buffer
      }
   }
   else
   {
      //there is not enough bytes in buffer to parse header
   }
   return 0;
}

/**
 * callback for fileManager when it requests a send buffer
 * returns pointer to allocated buffer, or NULL on failure
 */
static uint8_t *apx_clientConnection_getSendBuffer(void *arg, int32_t msgLen)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if ( (self != 0) && (msgLen>0) )
   {
      int8_t result=0;
      int32_t requestedLen;
      //create a buffer where we have room to encode the message header (the length of the message) in addition to the user requested length
      int32_t currentLen = adt_bytearray_length(&self->sendBuffer);
      requestedLen = msgLen + self->maxMsgHeaderSize;
      if (currentLen<requestedLen)
      {
         result = adt_bytearray_resize(&self->sendBuffer, (uint32_t) requestedLen);
      }
      if (result == 0)
      {
         uint8_t *data = adt_bytearray_data(&self->sendBuffer);
         assert(data != 0);
         return &data[self->maxMsgHeaderSize]; //return a pointer directly after the message header size.
      }
   }
   return (uint8_t*) 0;
}

/**
 * callback for fileManager when it requests to send buffer (which was previously returned by apx_serverConnection_getSendBuffer
 */
static int32_t apx_clientConnection_send(void *arg, int32_t offset, int32_t msgLen)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if ( (self != 0) && (offset>=0) && (msgLen>=0) )
   {
      int32_t sendBufferLen;
      uint8_t *sendBuffer = adt_bytearray_data(&self->sendBuffer);
      sendBufferLen = adt_bytearray_length(&self->sendBuffer);
      if ((sendBuffer != 0) && (msgLen+self->maxMsgHeaderSize<=sendBufferLen) )
      {
         uint8_t header[sizeof(uint32_t)];
         uint8_t headerLen;
         uint8_t *headerEnd;
         uint8_t *pBegin;
         if (self->maxMsgHeaderSize == (uint8_t) sizeof(uint32_t))
         {
            headerEnd = headerutil_numEncode32(header, (uint32_t) sizeof(header), msgLen);
            if (headerEnd>header)
            {
               headerLen=headerEnd-header;
            }
            else
            {
               assert(0);
               return -1; //header buffer too small
            }
         }
         else
         {
            return -1; //not yet implemented
         }
         //place header just before user data begin
         pBegin = sendBuffer+(self->maxMsgHeaderSize+offset-headerLen); //the part in the parenthesis is where the user data begins
         memcpy(pBegin, header, headerLen);         
         SOCKET_SEND(self->socketObject, pBegin, msgLen+headerLen);
         return 0;
      }
      else
      {
         assert(0);
      }
   }
   return -1;
}

//secondary transmitHandler interface
static uint8_t* apx_clientConnection_getSendBufferRaw(void *arg, int32_t dataLen)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if ( (self != 0) && (dataLen>0) )
   {
      int8_t result=0;
      int32_t currentLen = adt_bytearray_length(&self->sendBuffer);
      if (currentLen<dataLen)
      {
         result = adt_bytearray_resize(&self->sendBuffer, (uint32_t) dataLen);
      }
      if (result == 0)
      {
         uint8_t *data = adt_bytearray_data(&self->sendBuffer);
         assert(data != 0);
         return &data[0]; //return a pointer directly after the message header size.
      }
   }
   return (uint8_t*) 0;
}

static int32_t apx_clientConnection_sendRaw(void *arg, int32_t dataLen)
{
   apx_clientConnection_t *self = (apx_clientConnection_t*) arg;
   if ( (self != 0) && (dataLen>0) )
   {
      bool isConnected;
      SPINLOCK_ENTER(self->lock);
      isConnected = self->isConnected;
      SPINLOCK_LEAVE(self->lock);
      if (isConnected)
      {
         int32_t sendBufferLen;
         uint8_t *sendBuffer = adt_bytearray_data(&self->sendBuffer);
         sendBufferLen = adt_bytearray_length(&self->sendBuffer);
         if (dataLen > sendBufferLen)
         {
            return -1;
         }
         SOCKET_SEND(self->socketObject, sendBuffer, dataLen);
      }
      else
      {
         return 0;
      }
   }
   return -1;
}


static int8_t apx_clientConnection_dataReceived(apx_clientConnection_t *self, const uint8_t *dataBuf, uint32_t dataLen, uint32_t *parseLen)
{
   if ( (self != 0) && (dataBuf != 0) )
   {
      uint32_t totalParseLen = 0;
      uint32_t remain = dataLen;
      int8_t result = 0;
      const uint8_t *pNext = dataBuf;
      while(totalParseLen<dataLen)
      {
         uint32_t internalParseLen = 0;
         result=apx_clientConnection_parseMessage(self, pNext, remain, &internalParseLen);
         if ( (result == 0) && (internalParseLen!=0) )
         {
            assert(internalParseLen<=dataLen);
            pNext+=internalParseLen;
            totalParseLen+=internalParseLen;
            remain-=internalParseLen;
         }
         else
         {
            break;
         }
      }
      //no more complete messages can be parsed. There may be a partial message left in buffer, but we ignore it until more data has been recevied.
      *parseLen = totalParseLen;
      return result;
   }
   return -1;
}
