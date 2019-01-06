//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "apx_nodeData.h"
#include "apx_file2.h"
#include "rmf.h"
#ifdef APX_EMBEDDED
#include "apx_es_fileManager.h"
#else
#include <malloc.h>
#include <assert.h>
#include "apx_fileManager.h"
#include "apx_node.h"
#include "apx_portDataMap.h"
#include "apx_parser.h"
#endif
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_NODE_INVALID_OFFSET 0xFFFFFFFFu



//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static int8_t apx_nodeData_createFileInfo(apx_nodeData_t *self, rmf_fileInfo_t *fileInfo, uint8_t fileType);
static const char *apx_nodeData_getFileExtenstionFromType(uint8_t fileType);
static uint32_t apx_nodeData_getFileLengthFromType(apx_nodeData_t *self, uint8_t fileType);
static apx_error_t apx_nodeData_writeDefinitionFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more);
static apx_error_t apx_nodeData_writeInPortDataFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more);
static apx_error_t apx_nodeData_writeOutPortDataFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more);
static apx_error_t apx_nodeData_readDefinitionFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len);
static apx_error_t apx_nodeData_readInPortDataFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len);
static apx_error_t apx_nodeData_readOutPortDataFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len);
static void apx_nodeData_triggerDefinitionDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len);
static void apx_nodeData_triggerInPortDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len);
static void apx_nodeData_triggerOutPortDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len);
static apx_error_t apx_nodeData_createInitData(apx_nodeData_t *self);

#ifndef APX_EMBEDDED
static void apx_nodeData_clearPortDataBuffers(apx_nodeData_t *self);
#endif

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void apx_nodeData_create(apx_nodeData_t *self, const char *name, uint8_t *definitionBuf, uint32_t definitionDataLen, uint8_t *inPortDataBuf, uint8_t *inPortDirtyFlags, uint32_t inPortDataLen, uint8_t *outPortDataBuf, uint8_t *outPortDirtyFlags, uint32_t outPortDataLen)
{
   if (self != 0)
   {
      self->isRemote = false; //default false, used by nodeManager to determine whether this belongs to a remote or a local node
      self->isWeakref = true; //default true, all pointers in this object are weak references (will not be automatically deleted when this object is destroyed)
      self->name=name;
      self->definitionDataBuf = definitionBuf;
      self->definitionDataLen = definitionDataLen;
      self->inPortDataBuf = inPortDataBuf;
      self->inPortDataLen = inPortDataLen;
      self->inPortDirtyFlags = inPortDirtyFlags;
      self->outPortDataBuf = outPortDataBuf;
      self->outPortDataLen = outPortDataLen;
      self->outPortDirtyFlags = outPortDirtyFlags;
      self->inPortConnectionCount = (apx_connectionCount_t*) 0;
      self->outPortConnectionCount = (apx_connectionCount_t*) 0;
      self->inPortConnectionCountTotal = 0u;
      self->outPortConnectionCountTotal = 0u;
      self->numInPorts = 0;
      self->numOutPorts = 0;
      self->outPortDataFile = (apx_file2_t*) 0;
      self->inPortDataFile = (apx_file2_t*) 0;
      self->definitionFile = (apx_file2_t*) 0;
      self->apxNodeWriteCbk = (apx_nodeData_nodeCbkFunc*) 0;
      self->checksumType = APX_CHECKSUM_NONE;
      self->definitionStartOffset = APX_NODE_INVALID_OFFSET;
      self->inPortDataStartOffset = APX_NODE_INVALID_OFFSET;
      self->outPortDataStartOffset = APX_NODE_INVALID_OFFSET;
      memset(&self->checksumData[0], 0, sizeof(self->checksumData));
      apx_nodeData_setEventListener(self, (apx_nodeDataEventListener_t*) 0);
#ifdef APX_EMBEDDED
      self->fileManager = (apx_es_fileManager_t*) 0;
#else
      SPINLOCK_INIT(self->inPortDataLock);
      SPINLOCK_INIT(self->outPortDataLock);
      SPINLOCK_INIT(self->definitionDataLock);
      SPINLOCK_INIT(self->internalLock);
      self->portDataMap = (apx_portDataMap_t*) 0;
      self->fileManager = (apx_fileManager_t*) 0;
      self->node = (apx_node_t*) 0;
#endif
   }
}

void apx_nodeData_destroy(apx_nodeData_t *self)
{
   if (self != 0)
   {
#ifndef APX_EMBEDDED
      SPINLOCK_DESTROY(self->inPortDataLock);
      SPINLOCK_DESTROY(self->outPortDataLock);
      SPINLOCK_DESTROY(self->definitionDataLock);
      SPINLOCK_DESTROY(self->internalLock);

      if (self->isWeakref == false)
      {
         //pointers are strongly referenced when isWeakRef is false. delete all pointers that are not NULL
         if ( self->name != 0)
         {
            free((char*)self->name);
         }
         if ( self->definitionDataBuf != 0)
         {
            free(self->definitionDataBuf);
         }
         if (self->portDataMap != 0)
         {
            apx_portDataMap_delete(self->portDataMap);
         }
         apx_nodeData_clearPortDataBuffers(self);
      }
      if ( self->node != 0)
      {
         apx_node_delete(self->node);
      }
#endif
   }
}
#ifndef APX_EMBEDDED
/**
 * creates a new apx_nodeData_t with all pointers set to NULL
 */
apx_nodeData_t *apx_nodeData_new(uint32_t definitionDataLen)
{
   apx_nodeData_t *self = 0;
   self = (apx_nodeData_t*) malloc(sizeof(apx_nodeData_t));
   if (self != 0)
   {
      uint8_t *definitionDataBuf = (uint8_t*) 0;
      if (definitionDataLen > 0)
      {
         definitionDataBuf = (uint8_t*) malloc(definitionDataLen);
         if (definitionDataBuf == 0)
         {
            free(self);
            return (apx_nodeData_t*) 0;
         }
      }
      apx_nodeData_create(self, 0, definitionDataBuf, definitionDataLen, 0, 0, 0, 0, 0, 0);
      self->isWeakref = false;
   }
   else
   {
      errno = ENOMEM;
   }
   return self;
}
#endif


bool apx_nodeData_isOutPortDataOpen(apx_nodeData_t *self)
{
   bool retval = false;
   if (self != 0)
   {
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->internalLock);
#endif

      if ( (self->fileManager != 0) && (self->outPortDataFile != 0) && (self->outPortDataFile->isOpen == true) )
      {
         retval = true;
      }

#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->internalLock);
#endif
   }
   return retval;
}

void apx_nodeData_setEventListener(apx_nodeData_t *self, apx_nodeDataEventListener_t *eventListener)

{
   if (self != 0)
   {
      if (eventListener == (apx_nodeDataEventListener_t*) 0)
      {
         memset(&self->eventListener, 0, sizeof(self->eventListener));
      }
      else
      {
         memcpy(&self->eventListener, eventListener, sizeof(self->eventListener));
      }
   }
}

void apx_nodeData_setApxNodeCallback(apx_nodeData_t *self, apx_nodeData_nodeCbkFunc* cbk)
{
   if (self != 0)
   {
      self->apxNodeWriteCbk = cbk;
   }
}

#ifndef APX_EMBEDDED
void apx_nodeData_delete(apx_nodeData_t *self)
{
   if (self != 0)
   {
      apx_nodeData_destroy(self);
      free(self);
   }
}

void apx_nodeData_vdelete(void *arg)
{
   apx_nodeData_delete((apx_nodeData_t*) arg);
}

/**
 * Helper functions that fully creates an apx_nodeData structure from an APX text string.
 */
apx_nodeData_t *apx_nodeData_makeFromString(struct apx_parser_tag *parser, const char* apx_text)
{
   apx_node_t *node = apx_parser_parseString(parser, apx_text);
   if (node != 0)
   {
      apx_error_t errorCode = APX_NO_ERROR;
      apx_nodeData_t *nodeData;
      uint32_t definitionLen = (uint32_t) strlen(apx_text);
      nodeData = apx_nodeData_new(definitionLen);
      if (nodeData != 0){
         apx_parser_clearNodes(parser);
         apx_nodeData_setNode(nodeData, node);
         errorCode = apx_nodeData_createPortDataBuffers(nodeData);
         if (errorCode == APX_NO_ERROR)
         {
            apx_portDataMap_t *portDataMap = apx_portDataMap_new(nodeData);
            if (portDataMap != 0)
            {
               apx_nodeData_setPortDataMap(nodeData, portDataMap);
            }
            else
            {
               apx_nodeData_delete(nodeData);
               nodeData = (apx_nodeData_t*) 0;
            }
            assert((nodeData->definitionDataBuf != 0) && (nodeData->definitionDataLen == definitionLen));
            memcpy(nodeData->definitionDataBuf, apx_text, definitionLen);
         }
         else
         {
            apx_setError(errorCode);
            apx_nodeData_delete(nodeData);
            nodeData = (apx_nodeData_t*) 0;
         }
      }
      return nodeData;
   }
   return (apx_nodeData_t*) 0;
}

#endif

apx_error_t apx_nodeData_setChecksumData(apx_nodeData_t *self, uint8_t checksumType, uint8_t *checksumData)
{
   if ( (self != 0) && (checksumType <= APX_CHECKSUM_SHA256) )
   {
      self->checksumType = checksumType;
      if (checksumType == APX_CHECKSUM_SHA256)
      {
         if (checksumData == 0)
         {
            return APX_INVALID_ARGUMENT_ERROR;
         }
         memcpy(&self->checksumData[0], checksumData, APX_CHECKSUMLEN_SHA256);
      }
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

apx_error_t apx_nodeData_readDefinitionData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   if( (self != 0) && (dest != 0) )
   {
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->definitionDataLock);
#endif
      if ( (offset+len) > self->definitionDataLen)
      {
         retval = APX_INVALID_ARGUMENT_ERROR;
      }
      else
      {
         memcpy(dest, &self->definitionDataBuf[offset], len);
      }
#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->definitionDataLock);
#endif
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

apx_error_t apx_nodeData_readOutPortData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   if( (self != 0) && (dest != 0) )
   {
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->outPortDataLock);
#endif
      if ( (offset+len) > self->outPortDataLen)
      {
         retval = APX_INVALID_ARGUMENT_ERROR;
      }
      else
      {
         memcpy(dest, &self->outPortDataBuf[offset], len);
         self->outPortDirtyFlags[offset]=0;
      }
#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->outPortDataLock);
#endif
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

apx_error_t apx_nodeData_readInPortData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   if( (self != 0) && (dest != 0) )
   {
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->inPortDataLock);
#endif
      if ( (offset+len) > self->inPortDataLen)
      {
         retval = APX_INVALID_ARGUMENT_ERROR;
      }
      else
      {
         memcpy(dest, &self->inPortDataBuf[offset], len);
         self->inPortDirtyFlags[offset]=0;
      }
#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->inPortDataLock);
#endif
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

void apx_nodeData_lockOutPortData(apx_nodeData_t *self)
{
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->outPortDataLock);
#endif
}

void apx_nodeData_unlockOutPortData(apx_nodeData_t *self)
{
#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->outPortDataLock);
#endif
}

void apx_nodeData_lockInPortData(apx_nodeData_t *self)
{
#ifndef APX_EMBEDDED
      SPINLOCK_ENTER(self->inPortDataLock);
#endif
}

void apx_nodeData_unlockInPortData(apx_nodeData_t *self)
{
#ifndef APX_EMBEDDED
      SPINLOCK_LEAVE(self->inPortDataLock);
#endif
}

void apx_nodeData_outPortDataNotify(apx_nodeData_t *self, apx_offset_t offset, apx_size_t length)
{
   if (self != 0)
   {
      if ( (self->fileManager != 0) && (self->outPortDataFile != 0) && (self->outPortDataFile->isOpen == true) )
      {
#ifdef APX_EMBEDDED
         apx_es_fileManager_onFileUpdate(self->fileManager, self->outPortDataFile, offset, length);
#else
#if 0
         apx_fileManager_triggerFileUpdatedEvent(self->fileManager, self->outPortDataFile, offset, length);
#endif
#endif
      }
   }
}

apx_error_t apx_nodeData_writeInPortData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
#ifndef APX_EMBEDDED
   SPINLOCK_ENTER(self->inPortDataLock);
#endif
   if ( (offset+len) > self->inPortDataLen) //attempted write outside bounds
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   else
   {
      memcpy(&self->inPortDataBuf[offset], src, len);
   }
#ifndef APX_EMBEDDED
   SPINLOCK_LEAVE(self->inPortDataLock);
#endif
   return retval;

}

apx_error_t apx_nodeData_writeOutPortData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
#ifndef APX_EMBEDDED
   SPINLOCK_ENTER(self->outPortDataLock);
#endif
   if ( (offset+len) > self->outPortDataLen)
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   else
   {
      memcpy(&self->outPortDataBuf[offset], src, len);
   }
#ifndef APX_EMBEDDED
   SPINLOCK_LEAVE(self->outPortDataLock);
#endif
   return retval;
}

apx_error_t apx_nodeData_writeDefinitionData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
#ifndef APX_EMBEDDED
   SPINLOCK_ENTER(self->definitionDataLock);
#endif
   if ( (offset+len) > self->definitionDataLen)
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   else
   {
      memcpy(&self->definitionDataBuf[offset], src, len);
   }
#ifndef APX_EMBEDDED
   SPINLOCK_LEAVE(self->definitionDataLock);
#endif
   return retval;
}

void apx_nodeData_setInPortDataFile(apx_nodeData_t *self, struct apx_file2_tag *file)
{
   if (self != 0)
   {
      apx_file_handler_t handler;
      handler.arg = (void*) self;
      handler.read = apx_nodeData_readInPortDataFile;
      handler.write = apx_nodeData_writeInPortDataFile;
      apx_file2_setHandler(file, &handler);
      self->inPortDataFile = file;
   }
}

void apx_nodeData_setOutPortDataFile(apx_nodeData_t *self, struct apx_file2_tag *file)
{
   if (self != 0)
   {
      apx_file_handler_t handler;
      handler.arg = (void*) self;
      handler.read = apx_nodeData_readOutPortDataFile;
      handler.write = apx_nodeData_writeOutPortDataFile;
      apx_file2_setHandler(file, &handler);
      self->outPortDataFile = file;
   }
}

void apx_nodeData_setDefinitionFile(apx_nodeData_t *self, struct apx_file2_tag *file)
{
   if ( (self != 0) && (file != 0) )
   {
      apx_file_handler_t handler;
      handler.arg = (void*) self;
      handler.read = apx_nodeData_readDefinitionFile;
      handler.write = apx_nodeData_writeDefinitionFile;
      apx_file2_setHandler(file, &handler);
      self->definitionFile = file;
   }
}

struct apx_file2_tag *apx_nodeData_getDefinitionFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->definitionFile;
   }
   return (struct apx_file2_tag *) 0;
}

struct apx_file2_tag *apx_nodeData_getInPortDataFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->inPortDataFile;
   }
   return (struct apx_file2_tag *) 0;
}

struct apx_file2_tag *apx_nodeData_getOutPortDataFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->outPortDataFile;
   }
   return (struct apx_file2_tag *) 0;
}

//Connection count API
apx_connectionCount_t apx_nodeData_getInPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   apx_connectionCount_t retval = 0;
   if ( (self != 0) && (self->inPortConnectionCount != 0) && (portId < self->numInPorts) )
   {
      retval = self->inPortConnectionCount[portId];
   }
   return retval;
}

apx_connectionCount_t apx_nodeData_getOutPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   apx_connectionCount_t retval = 0;
   if ( (self != 0) && (self->outPortConnectionCount != 0) && (portId < self->numOutPorts) )
   {
      retval = self->outPortConnectionCount[portId];
   }
   return retval;
}

void apx_nodeData_incInPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->inPortConnectionCount != 0) && (portId < self->numInPorts) )
   {
      if (self->inPortConnectionCount[portId] < APX_CONNECTION_COUNT_MAX)
      {
         self->inPortConnectionCount[portId]++;
      }
   }
}

void apx_nodeData_incOutPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->outPortConnectionCount != 0) && (portId < self->numOutPorts) )
   {
      if (self->outPortConnectionCount[portId] < APX_CONNECTION_COUNT_MAX)
      {
         self->outPortConnectionCount[portId]++;
      }
   }
}

void apx_nodeData_decInPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->inPortConnectionCount != 0) && (portId < self->numInPorts) )
   {
      if (self->inPortConnectionCount[portId] > 0u)
      {
         self->inPortConnectionCount[portId]--;
      }
   }
}

void apx_nodeData_decOutPortConnectionCount(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->outPortConnectionCount != 0) && (portId < self->numOutPorts) )
   {
      if (self->outPortConnectionCount[portId] > 0u)
      {
         self->outPortConnectionCount[portId]--;
      }
   }
}


#ifndef APX_EMBEDDED

/**
 * Creates a new APX definition file based on contents on length of definition buffer
 */
struct apx_file2_tag *apx_nodeData_createLocalDefinitionFile(apx_nodeData_t *self)
{
   if ( (self != 0) && (self->definitionDataBuf != 0) && (self->definitionDataLen > 0) )
   {
      rmf_fileInfo_t info;
      apx_file2_t *definitionFile = (apx_file2_t*) 0;
      char fileName[RMF_MAX_FILE_NAME+1];
      const char *nodeName;
      apx_file_handler_t handler = {0, apx_nodeData_readDefinitionFile, apx_nodeData_writeDefinitionFile};
      handler.arg = (void*) self;
      nodeName = apx_nodeData_getName(self);
      if (nodeName != 0)
      {
         strcpy(fileName, nodeName);
         strcat(fileName, APX_DEFINITION_FILE_EXT);
         rmf_fileInfo_create(&info, fileName, RMF_INVALID_ADDRESS, self->definitionDataLen, RMF_FILE_TYPE_FIXED);
         definitionFile = apx_file2_newLocal(&info, &handler);
         if (definitionFile != 0)
         {
            self->definitionFile = definitionFile;
         }
      }
      return definitionFile;
   }
   return (apx_file2_t*) 0;
}

/**
 * Creates a new APX out port data file based on length of out port data buffer
 */
struct apx_file2_tag *apx_nodeData_createLocalOutPortDataFile(apx_nodeData_t *self)
{
   if ( (self != 0) && (self->outPortDataBuf != 0) && (self->outPortDataLen > 0) )
   {
      rmf_fileInfo_t info;
      apx_file2_t *outPortDataFile = (apx_file2_t*) 0;
      char fileName[RMF_MAX_FILE_NAME+1];
      const char *nodeName;
      apx_file_handler_t handler = {0, apx_nodeData_readOutPortDataFile, apx_nodeData_writeOutPortDataFile};
      handler.arg = (void*) self;
      nodeName = apx_nodeData_getName(self);
      if (nodeName != 0)
      {
         strcpy(fileName, nodeName);
         strcat(fileName, APX_OUTDATA_FILE_EXT);
         rmf_fileInfo_create(&info, fileName, RMF_INVALID_ADDRESS, self->outPortDataLen, RMF_FILE_TYPE_FIXED);
         outPortDataFile = apx_file2_newLocal(&info, &handler);
         if (outPortDataFile != 0)
         {
            self->outPortDataFile = outPortDataFile;
         }
      }
      return outPortDataFile;
   }
   return (apx_file2_t*) 0;
}

apx_error_t apx_nodeData_createPortDataBuffers(apx_nodeData_t *self)
{
   if ( (self != 0) && (self->isWeakref == false) && (self->node != 0) )
   {
      self->outPortDataLen = apx_node_calcOutPortDataLen(self->node);
      self->inPortDataLen = apx_node_calcInPortDataLen(self->node);
      if ( (self->outPortDataLen<0) || (self->inPortDataLen<0) )
      {
         return APX_LENGTH_ERROR;
      }
      if (self->outPortDataLen > 0)
      {
         int32_t numProvidePorts = apx_node_getNumProvidePorts(self->node);
         assert(numProvidePorts > 0);
         size_t connectionCountSize = sizeof(apx_connectionCount_t)*numProvidePorts;
         self->numOutPorts = numProvidePorts;
         self->outPortDataBuf = (uint8_t*) malloc(self->outPortDataLen);
         self->outPortDirtyFlags = (uint8_t*) malloc(self->outPortDataLen);
         self->outPortConnectionCount = (apx_connectionCount_t*) malloc(connectionCountSize);
         if ( (self->outPortDataBuf == 0) || (self->outPortDirtyFlags == 0) || (self->outPortConnectionCount == 0) )
         {
            apx_nodeData_clearPortDataBuffers(self);
            return APX_MEM_ERROR;
         }
         memset(self->outPortConnectionCount, 0, connectionCountSize);
      }
      if (self->inPortDataLen > 0)
      {
         apx_error_t result;
         int32_t numRequirePorts = apx_node_getNumRequirePorts(self->node);
         assert(numRequirePorts > 0);
         size_t connectionCountSize = sizeof(apx_connectionCount_t)*numRequirePorts;
         self->numInPorts = numRequirePorts;
         self->inPortDataBuf = (uint8_t*) malloc(self->inPortDataLen);
         self->inPortDirtyFlags = (uint8_t*) malloc(self->inPortDataLen);
         self->inPortConnectionCount = (apx_connectionCount_t*) malloc(connectionCountSize);
         if ( (self->inPortDataBuf == 0) || (self->inPortDirtyFlags == 0) || (self->inPortConnectionCount == 0))
         {
            apx_nodeData_clearPortDataBuffers(self);
            return APX_MEM_ERROR;
         }
         memset(self->inPortConnectionCount, 0, connectionCountSize);
         result = apx_nodeData_createInitData(self);
         if (result != APX_NO_ERROR)
         {
            apx_nodeData_clearPortDataBuffers(self);
            return result;
         }
      }
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

apx_error_t apx_nodeData_createPortDataMap(apx_nodeData_t *self, uint8_t mode)
{
   if ( (self != 0) && (self->node != 0) )
   {
      apx_error_t result = APX_NO_ERROR;
      self->portDataMap = apx_portDataMap_new(self);
      if (self->portDataMap == (apx_portDataMap_t*) 0)
      {
         return APX_MEM_ERROR;
      }
      if (mode == APX_SERVER_MODE)
      {
         if (self->outPortDataLen > 0)
         {
            result = apx_portDataMap_initProvidePortByteMap(self->portDataMap, self->node);
            if (result == APX_NO_ERROR)
            {
               result = apx_portDataMap_initPortTriggerList(self->portDataMap);
            }
         }
      }
      else //APX_CLIENT_MODE
      {
         if (self->inPortDataLen > 0)
         {
            result = apx_portDataMap_initRequirePortByteMap(self->portDataMap, self->node);
         }
      }
      if (result != APX_NO_ERROR)
      {
         apx_portDataMap_delete(self->portDataMap);
         self->portDataMap = (apx_portDataMap_t*) 0;
      }
      return result;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}

void apx_nodeData_setNode(apx_nodeData_t *self, struct apx_node_tag *node)
{
   if ( (self != 0) && (node != 0) )
   {
      self->node = node;
   }
}

struct apx_node_tag *apx_nodeData_getNode(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->node;
   }
   return (struct apx_node_tag*) 0;
}

void apx_nodeData_setPortDataMap(apx_nodeData_t *self, struct apx_portDataMap_tag *portDataMap)
{
   if ( (self != 0) && (portDataMap != 0) )
   {
      self->portDataMap = portDataMap;
   }
}

struct apx_portDataMap_tag* apx_nodeData_getPortDataMap(apx_nodeData_t *self)
{
   if ( self != 0 )
   {
      return self->portDataMap;
   }
   return (struct apx_portDataMap_tag*) 0;
}

struct apx_portDataRef_tag *apx_nodeData_getRequirePortDataRef(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->portDataMap != 0) )
   {
      return apx_portDataMap_getRequirePortData(self->portDataMap, portId);
   }
   return (struct apx_portDataRef_tag*) 0;
}

struct apx_portDataRef_tag *apx_nodeData_getProvidePortDataRef(apx_nodeData_t *self, apx_portId_t portId)
{
   if ( (self != 0) && (self->portDataMap != 0) )
   {
      return apx_portDataMap_getProvidePortData(self->portDataMap, portId);
   }
   return (struct apx_portDataRef_tag*) 0;
}


const char *apx_nodeData_getName(apx_nodeData_t *self)
{
   if (self != 0)
   {
      const char *name = self->name;
#ifdef APX_EMBEDDED
      return self->name;
#else
      if ( (name == 0) && (self->node != 0) )
      {
         name = apx_node_getName(self->node);
      }
      if ( (name == 0) && (self->definitionFile != 0) )
      {
         name = apx_file2_basename(self->definitionFile);
      }
      return name;
#endif
   }
   return (const char*) 0;
}




void apx_nodeData_setConnection(apx_nodeData_t *self, struct apx_connectionBase_tag *connection)
{
   if ( (self != 0) && (connection != 0) )
   {
      self->connection = connection;
   }
}

struct apx_connectionBase_tag* apx_nodeData_getConnection(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->connection;
   }
   return (struct apx_connectionBase_tag*) 0;
}

/**
 * Returns true when these criteria are met:
 * 1. The <node>.apx file has been received (or found in cache) and successfully parsed
 * 2. If the APX node has provide ports the <node>.out file has been opened and fully received (sent from client to server)
 * 3. If the APX node has require ports the <node>.in file has been requested to be opened (sent from server to client)
 */
bool apx_nodeData_isComplete(apx_nodeData_t *self)
{
   bool retval = true;
   if (self != 0)
   {
      if ( (self->node != 0) && (self->portDataMap != 0)) //Test 1
      {
         if (self->portDataMap->numProvidePorts > 0 )
         {
            if ( (self->outPortDataFile == 0) || (apx_file2_isOpen(self->outPortDataFile)==false) || (apx_file2_isDataValid(self->outPortDataFile) == false) )
            {
               retval = false;
            }
         }
         if ( (retval == true) && (self->portDataMap->numRequirePorts > 0))
         {
            if ( (self->inPortDataFile == 0) || (apx_file2_isOpen(self->inPortDataFile)==false) )
            {
               retval = false;
            }
         }
      }
      else
      {
         retval = false;
      }
   }
   else
   {
      retval = false;
   }
   return retval;
}


#endif //!APX_EMBEDDED

uint32_t apx_nodeData_getInPortDataLen(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->inPortDataLen;
   }
   return 0;
}

uint32_t apx_nodeData_getOutPortDataLen(apx_nodeData_t *self)
{
   if (self != 0)
   {
      return self->outPortDataLen;
   }
   return 0;
}


#ifdef APX_EMBEDDED
void apx_nodeData_setFileManager(apx_nodeData_t *self, struct apx_es_fileManager_tag *fileManager)
#else
void apx_nodeData_setFileManager(apx_nodeData_t *self, struct apx_fileManager_tag *fileManager)
#endif
{
   if (self != 0)
   {
      self->fileManager = fileManager;
   }
}

#if 0
void apx_nodeData_triggerInPortDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len)
{   
   if ( (self != 0) && (self->apxNodeWriteCbk != 0) )
   {
      self->apxNodeWriteCbk(offset, len);
   }
}
#endif

apx_file2_t *apx_nodeData_newLocalDefinitionFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      const uint8_t fileType = APX_DEFINITION_FILE;
      rmf_fileInfo_t fileInfo;
      int8_t result;
      result = apx_nodeData_createFileInfo(self, &fileInfo, fileType);
      if (result == 0)
      {
         self->definitionFile = apx_file2_newLocal(&fileInfo, NULL);
         return self->definitionFile;
      }
   }
   return (apx_file2_t*) 0;
}

struct apx_file2_tag *apx_nodeData_newLocalOutPortDataFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      const uint8_t fileType = APX_OUTDATA_FILE;
      rmf_fileInfo_t fileInfo;
      int8_t result;
      result = apx_nodeData_createFileInfo(self, &fileInfo, fileType);
      if (result == 0)
      {
         self->outPortDataFile = apx_file2_newLocal(&fileInfo, NULL);
         return self->outPortDataFile;
      }
   }
   return (apx_file2_t*) 0;
}

struct apx_file2_tag *apx_nodeData_newLocalInPortDataFile(apx_nodeData_t *self)
{
   if (self != 0)
   {
      const uint8_t fileType = APX_INDATA_FILE;
      rmf_fileInfo_t fileInfo;
      int8_t result;
      result = apx_nodeData_createFileInfo(self, &fileInfo, fileType);
      if (result == 0)
      {
         self->inPortDataFile = apx_file2_newLocal(&fileInfo, NULL);
         return self->inPortDataFile;
      }
   }
   return (apx_file2_t*) 0;
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static int8_t apx_nodeData_createFileInfo(apx_nodeData_t *self, rmf_fileInfo_t *fileInfo, uint8_t fileType)
{
   if ( (self != 0) && (fileInfo != 0) && (fileType >= APX_OUTDATA_FILE ) && (fileType <= APX_DEFINITION_FILE))
   {
      size_t nameLen;
      const char *fileExtName;
      uint32_t fileLen;
      char name[RMF_MAX_FILE_NAME+1];
      nameLen = strlen(apx_nodeData_getName(self));

      errno = 0;
      fileExtName = apx_nodeData_getFileExtenstionFromType(fileType);
      fileLen = apx_nodeData_getFileLengthFromType(self, fileType);
      if ( ( (nameLen+APX_MAX_FILE_EXT_LEN) <= RMF_MAX_FILE_NAME) && (errno == 0) )
      {
         memcpy(name, self->name, nameLen);
         strcpy(name+nameLen, fileExtName);
         rmf_fileInfo_create(fileInfo, name, RMF_INVALID_ADDRESS, fileLen, RMF_FILE_TYPE_FIXED);
         return 0;
      }
      else
      {
         return -1;
      }
   }
   errno = EINVAL;
   return -1;
}

static const char *apx_nodeData_getFileExtenstionFromType(uint8_t fileType)
{
   const char *retval = (const char*) 0;
   switch(fileType)
   {
   case APX_OUTDATA_FILE:
      retval = APX_OUTDATA_FILE_EXT;
      break;
   case APX_INDATA_FILE:
      retval = APX_INDATA_FILE_EXT;
      break;
   case APX_DEFINITION_FILE:
      retval = APX_DEFINITION_FILE_EXT;
      break;
   default:
      errno = EINVAL;
   }
   return retval;
}

static uint32_t apx_nodeData_getFileLengthFromType(apx_nodeData_t *self, uint8_t fileType)
{
   uint32_t retval = 0;
   switch(fileType)
   {
   case APX_OUTDATA_FILE:
      retval = self->outPortDataLen;
      break;
   case APX_INDATA_FILE:
      retval = self->inPortDataLen;
      break;
   case APX_DEFINITION_FILE:
      retval = self->definitionDataLen;
      break;
   default:
      errno = EINVAL;
   }
   return retval;
}

static apx_error_t apx_nodeData_writeDefinitionFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->definitionFile == file) && (offset != APX_NODE_INVALID_OFFSET ) )
   {
      if ( self->definitionStartOffset == APX_NODE_INVALID_OFFSET )
      {
         self->definitionStartOffset = offset;
      }
      retval = apx_nodeData_writeDefinitionData(self, src, offset, len);
      if (more == false)
      {
         if ( retval == APX_NO_ERROR )
         {
            uint32_t totalWriteLength = offset + len - self->definitionStartOffset;
            apx_file2_setDataValid(file);
            apx_nodeData_triggerDefinitionDataWritten(self, self->definitionStartOffset, totalWriteLength);
         }
         self->definitionStartOffset = APX_NODE_INVALID_OFFSET;
      }
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static apx_error_t apx_nodeData_writeInPortDataFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->inPortDataFile == file) && (offset != APX_NODE_INVALID_OFFSET ) )
   {
      if ( self->inPortDataStartOffset == APX_NODE_INVALID_OFFSET )
      {
         self->inPortDataStartOffset = offset;
      }
      retval = apx_nodeData_writeInPortData(self, src, offset, len);
      if (more == false)
      {
         if ( retval == APX_NO_ERROR )
         {
            uint32_t totalWriteLength = offset + len - self->inPortDataStartOffset;
            apx_file2_setDataValid(file);
            apx_nodeData_triggerInPortDataWritten(self, self->inPortDataStartOffset, totalWriteLength);
         }
         self->inPortDataStartOffset = APX_NODE_INVALID_OFFSET;
      }
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static apx_error_t apx_nodeData_writeOutPortDataFile(void *arg, apx_file2_t *file, const uint8_t *src, uint32_t offset, uint32_t len, bool more)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->outPortDataFile == file) && (offset != APX_NODE_INVALID_OFFSET ) )
   {
      if ( self->outPortDataStartOffset == APX_NODE_INVALID_OFFSET )
      {
         self->outPortDataStartOffset = offset;
      }
      retval = apx_nodeData_writeOutPortData(self, src, offset, len);
      if (more == false)
      {
         if ( retval == APX_NO_ERROR )
         {
            uint32_t totalWriteLength = offset + len - self->outPortDataStartOffset;
            apx_file2_setDataValid(file);
            apx_nodeData_triggerOutPortDataWritten(self, self->outPortDataStartOffset, totalWriteLength);
         }
         self->outPortDataStartOffset = APX_NODE_INVALID_OFFSET;
      }
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static apx_error_t apx_nodeData_readDefinitionFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->definitionFile == file) )
   {
      retval = apx_nodeData_readDefinitionData(self, dest, offset, len);
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static apx_error_t apx_nodeData_readInPortDataFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->inPortDataFile == file) )
   {
      retval = apx_nodeData_readInPortData(self, dest, offset, len);
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static apx_error_t apx_nodeData_readOutPortDataFile(void *arg, struct apx_file2_tag *file, uint8_t *dest, uint32_t offset, uint32_t len)
{
   apx_error_t retval = APX_NO_ERROR;
   apx_nodeData_t *self = (apx_nodeData_t*) arg;
   if ( (self != 0) && (self->outPortDataFile == file) )
   {
      retval = apx_nodeData_readOutPortData(self, dest, offset, len);
   }
   else
   {
      retval = APX_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static void apx_nodeData_triggerDefinitionDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len)
{
   if (self->eventListener.definitionDataWritten != 0)
   {
      self->eventListener.definitionDataWritten(self->eventListener.arg, self, offset, len);
   }
}

static void apx_nodeData_triggerInPortDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len)
{
   if (self->eventListener.inPortDataWritten != 0)
   {
      self->eventListener.inPortDataWritten(self->eventListener.arg, self, offset, len);
   }
}

static void apx_nodeData_triggerOutPortDataWritten(apx_nodeData_t *self, uint32_t offset, uint32_t len)
{
   if (self->eventListener.outPortDataWritten != 0)
   {
      self->eventListener.outPortDataWritten(self->eventListener.arg, self, offset, len);
   }
}

static apx_error_t apx_nodeData_createInitData(apx_nodeData_t *self)
{
   if ( (self != 0) && (self->node != 0) && (self->inPortDataBuf != 0) && (self->inPortDataLen > 0) )
   {
      int32_t numRequirePorts;
      adt_bytearray_t *portData;
      uint8_t *pNext = self->inPortDataBuf;
      uint8_t *pEnd = self->inPortDataBuf+self->inPortDataLen;
      int32_t i;
      apx_node_t *node = self->node;
      portData = adt_bytearray_new(0);
      numRequirePorts = apx_node_getNumRequirePorts(node);
      for(i=0; i<numRequirePorts; i++)
      {
         int32_t packLen;
         int32_t dataLen;
         apx_port_t *port = apx_node_getRequirePort(node, i);
         assert(port != 0);
         packLen = apx_port_getPackLen(port);
         apx_node_fillPortInitData(node, port, portData);
         dataLen = adt_bytearray_length(portData);
         assert(packLen == dataLen);
         memcpy(pNext, adt_bytearray_data(portData), packLen);
         pNext+=packLen;
         assert(pNext<=pEnd);
      }
      assert(pNext==pEnd);
      adt_bytearray_delete(portData);
      return APX_NO_ERROR;
   }
   return APX_INVALID_ARGUMENT_ERROR;
}


#ifndef APX_EMBEDDED
static void apx_nodeData_clearPortDataBuffers(apx_nodeData_t *self)
{
   if ( self->inPortDataBuf != 0)
   {
      free(self->inPortDataBuf);
   }
   if ( self->outPortDataBuf != 0)
   {
      free(self->outPortDataBuf);
   }
   if ( self->inPortDirtyFlags != 0)
   {
      free(self->inPortDirtyFlags);
   }
   if ( self->outPortDirtyFlags != 0)
   {
      free(self->outPortDirtyFlags);
   }
   if ( self->inPortConnectionCount != 0)
   {
      free(self->inPortConnectionCount);
   }
   if ( self->outPortConnectionCount != 0)
   {
      free(self->outPortConnectionCount);
   }
}

#endif
