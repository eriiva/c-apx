
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#if defined(_MSC_PLATFORM_TOOLSET) && (_MSC_PLATFORM_TOOLSET<=110)
#include "msc_bool.h"
#else
#include <stdbool.h>
#endif
#include "adt_ary.h"
#include "adt_hash.h"
#include "apx_types.h"
#include "apx_nodeManager.h"
#include "apx_fileManager.h"
#include "apx_nodeData.h"
#include "apx_file2.h"
#include "apx_nodeInfo.h"
#include "apx_router.h"
#include "apx_logging.h" //internal logging
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) && (_MSC_VER<=1800)
#define snprintf _snprintf
#endif

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void apx_nodeManager_createNode(apx_nodeManager_t *self, const uint8_t *definitionBuf, int32_t definitionLen, struct apx_fileManager_tag *fileManager);
static void apx_nodeManager_setLocalNodeData(apx_nodeManager_t *self, apx_nodeData_t *nodeData);
static void apx_nodeManager_executePortTriggerFunction(const apx_dataTriggerFunction_t *triggerFunction, const apx_file2_t *file);
static void apx_nodeManager_removeRemoteNodeData(apx_nodeManager_t *self, apx_nodeData_t *nodeData);
static void apx_nodeManager_removeNodeInfo(apx_nodeManager_t *self, apx_nodeInfo_t *nodeInfo);
static bool apx_nodeManager_createInitData(apx_node_t *node, uint8_t *buf, int32_t bufLen);
static void apx_nodeManager_attachLocalNodesToFileManager(apx_nodeManager_t *self, apx_fileManager_t *fileManager);
static void apx_nodeManager_attachLocalNodeToFileManager(apx_nodeData_t *nodeData, apx_fileManager_t *fileManager);
static void apx_nodeManager_registerEventListenerToFileManager(apx_nodeManager_t *self, apx_fileManager_t *fileManager);
static void apx_nodeManager_remoteFileCreated(void *arg, struct apx_fileManager_tag *fileManager, apx_file2_t *remoteFile);
static void apx_nodeManager_remoteFileRevoked(void *arg, apx_fileManager_t *fileManager, apx_file2_t *remotefile);
static void apx_nodeManager_remoteFileWritten(void *arg, apx_fileManager_t *fileManager, const apx_file2_t *remoteFile, uint32_t offset, int32_t length);
//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void apx_nodeManager_create(apx_nodeManager_t *self)
{
   if (self != 0)
   {
      adt_hash_create(&self->nodeInfoMap, apx_nodeInfo_vdelete);
      self->router = (apx_router_t*) 0;
      self->debugMode = APX_DEBUG_NONE;
      adt_hash_create(&self->remoteNodeDataMap, apx_nodeData_vdelete);
      adt_hash_create(&self->localNodeDataMap, (void(*)(void*)) 0);
      MUTEX_INIT(self->mutex);
   }
}

void apx_nodeManager_destroy(apx_nodeManager_t *self)
{
   if(self != 0)
   {
      adt_hash_destroy(&self->nodeInfoMap);

      adt_hash_destroy(&self->remoteNodeDataMap);
      adt_hash_destroy(&self->localNodeDataMap);
      MUTEX_DESTROY(self->mutex);
   }
}

apx_nodeManager_t *apx_nodeManager_new(void)
{
   apx_nodeManager_t *self = (apx_nodeManager_t*) malloc(sizeof(apx_nodeManager_t));
   if(self != 0)
   {
      apx_nodeManager_create(self);
   }
   return self;
}

void apx_nodeManager_delete(apx_nodeManager_t *self)
{
   if(self != 0)
   {
      apx_nodeManager_destroy(self);
      free(self);
   }
}

/**
 * this is called by fileManager when a new file is seen
 */

/**
 * setter for self->router
 */
void apx_nodeManager_setRouter(apx_nodeManager_t *self, struct apx_router_tag *router)
{
   if ( (self != 0) )
   {
      self->router = router;
   }
}


/**
 * attaches local node to nodeManager.
 * It is expected that at least the definitionDataBuf is set to non-NULL pointer
 */
void apx_nodeManager_attachLocalNode(apx_nodeManager_t *self, apx_nodeData_t *nodeData)
{

   if ( (self != 0) && (nodeData != 0) )
   {
      MUTEX_LOCK(self->mutex);
      apx_nodeManager_setLocalNodeData(self, nodeData);
      MUTEX_UNLOCK(self->mutex);
   }
}

/**
 * attaches a fileManager to this nodeManager.
 * the fileManager pointer is stored as a weak reference inside a fileManagerList
 */
void apx_nodeManager_attachFileManager(apx_nodeManager_t *self, struct apx_fileManager_tag *fileManager)
{
   if ( (self != 0) && (fileManager != 0) )
   {
      apx_nodeManager_attachLocalNodesToFileManager(self, fileManager);
      if (apx_fileManager_isServerMode(fileManager) == true)
      {
         apx_nodeManager_registerEventListenerToFileManager(self, fileManager);
      }
   }
}

void apx_nodeManager_detachFileManager(apx_nodeManager_t *self, struct apx_fileManager_tag *fileManager)
{
   if ( (self != 0) && (fileManager != 0) )
   {
      int32_t i;
      int32_t numFiles;
      int32_t numNodes;
      adt_ary_t detachedFiles;
      adt_ary_t nodesToBeDeleted; //list of string pointers to apx_nodeData_t

      adt_ary_create(&detachedFiles, apx_file2_vdelete);
      adt_ary_create(&nodesToBeDeleted, apx_nodeData_vdelete);
#if 0
      apx_fileManager_stop(fileManager);
      apx_fileManager_detachFiles(fileManager, &detachedFiles); //this will transfer ownership of file objects to the detachedFiles list
#endif
      numFiles = adt_ary_length(&detachedFiles);
      for (i=0; i<numFiles; i++)
      {
#if 0 //TODO: REPLACE WITH SOMETHING MORE GENERIC
         apx_file2_t *file = (apx_file2_t*) adt_ary_value(&detachedFiles, i);
         if (file->nodeData != 0)
         {
            adt_ary_push_unique(&nodesToBeDeleted, file->nodeData);
         }
#endif
      }
      numNodes = adt_ary_length(&nodesToBeDeleted);
      for (i = 0; i < numNodes; i++)
      {
         apx_nodeData_t *nodeData = (apx_nodeData_t*)adt_ary_value(&nodesToBeDeleted, i);
         apx_nodeManager_removeRemoteNodeData(self, nodeData);
         if (nodeData->nodeInfo != 0)
         {
            apx_nodeInfo_t *nodeInfo = nodeData->nodeInfo;
            if (self->router != 0)
            {
               apx_router_detachNodeInfo(self->router, nodeInfo);
            }
            apx_nodeManager_removeNodeInfo(self, nodeInfo);
            apx_nodeInfo_delete(nodeInfo);
         }
      }
      adt_ary_destroy(&nodesToBeDeleted); //this will delete all nodeData objects using its virtual destructor.
      adt_ary_destroy(&detachedFiles); //this will delete all files using the virtual destructor.
   }
}

void apx_nodeManager_setDebugMode(apx_nodeManager_t *self, int8_t debugMode)
{
   if (self != 0)
   {
      self->debugMode=debugMode;
   }
}

struct apx_nodeData_tag *apx_nodeManager_findNodeData(apx_nodeManager_t *self, const char *name)
{
   if ( (self != 0) && (name != 0) )
   {
      void** ppVal;
      ppVal = adt_hash_get(&self->remoteNodeDataMap, name, 0);
      if (ppVal != 0)
      {
         return (apx_nodeData_t*) *ppVal;
      }
      ppVal = adt_hash_get(&self->localNodeDataMap, name, 0);
      if (ppVal != 0)
      {
         return (apx_nodeData_t*) *ppVal;
      }
   }
   return (struct apx_nodeData_tag*) 0;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

/**
 * used to create new remote nodes on server side
 */
static void apx_nodeManager_createNode(apx_nodeManager_t *self, const uint8_t *definitionBuf, int32_t definitionLen, struct apx_fileManager_tag *fileManager)
{
#if 0
   if( (self != 0) && (definitionBuf != 0) && (definitionLen > 0) )
   {
      int32_t numNodes;
      int32_t i;
#if 0
      char debugInfoStr[APX_DEBUG_INFO_MAX_LEN];
      debugInfoStr[0]=0;

      if (fileManager->debugInfo != 0)
      {
         snprintf(debugInfoStr, APX_DEBUG_INFO_MAX_LEN, " (%p)", fileManager->debugInfo);
      }
      APX_LOG_INFO("[APX_NODE_MANAGER]%s Server processing APX definition, len=%d", debugInfoStr, (int) definitionLen);
#endif

      apx_istream_reset(&self->apx_istream);
      apx_istream_open(&self->apx_istream);
      apx_istream_write(&self->apx_istream, definitionBuf, (uint32_t) definitionLen);
      apx_istream_close(&self->apx_istream);
      numNodes = apx_parser_getNumNodes(&self->parser);
      for (i=0;i<numNodes;i++)
      {
         apx_nodeInfo_t *nodeInfo;
         apx_node_t *apxNode = apx_parser_getNode(&self->parser, i);
         assert(apxNode != 0);
         apx_node_finalize(apxNode);
         nodeInfo = apx_nodeInfo_new(apxNode);
         if (nodeInfo != 0)
         {
            apx_nodeData_t *nodeData=0;
            char fileName[RMF_MAX_FILE_NAME];
            char *p;
            //int32_t inPortDataLen;
            int32_t outPortDataLen;


            nodeData = apx_nodeManager_findNodeData(self, apxNode->name);
            if (nodeData == 0)
            {
               APX_LOG_ERROR("[APX_NODE_MANAGER] %s", "Failed to create nodeData object");
               return;
            }
            apx_nodeData_setFileManager(nodeData,fileManager);
            apx_nodeData_setNodeInfo(nodeData, nodeInfo);
            apx_nodeInfo_setNodeData(nodeInfo, nodeData);
            nodeInfo->isWeakRef_node = false; //nodeInfo is now the owner of the node pointer (will trigger deletion when apx_nodeInfo_delete is called)
            adt_hash_set(&self->nodeInfoMap, apxNode->name, 0, nodeInfo);
            inPortDataLen = apx_nodeInfo_getInPortDataLen(nodeInfo);
            outPortDataLen = apx_nodeInfo_getOutPortDataLen(nodeInfo);
            
            //if node has output data, search a file called "<node_name>.out"
            if (outPortDataLen > 0)
            {
               apx_file2_t *outDataFile = 0;
               strcpy(fileName,apxNode->name);
               p=fileName+strlen(fileName);
               strcpy(p,".out");
#if 0
               outDataFile = apx_fileManager_findRemoteFile(fileManager, fileName);
#endif
               if (outDataFile != 0)
               {
                  //check if length of file is the expected length of our outPortDataLen calculation
                  if (outPortDataLen != (int32_t) outDataFile->fileInfo.length)
                  {
                     APX_LOG_ERROR("[APX_NODE_MANAGER] length of file %s is %d, expected length was %d\n", fileName, outDataFile->fileInfo.length, outPortDataLen);
                  }
                  else
                  {
#if 0 //TODO: REPLACE WITH SOMETHING MORE GENERIC
                     if (outDataFile->nodeData==0)
                     {
                        outDataFile->nodeData=nodeData;
                        //now create memory for the outPortData
                        nodeData->outPortDataBuf = (uint8_t*) malloc(outPortDataLen);
                        assert(nodeData->outPortDataBuf);
                        nodeData->outPortDirtyFlags = (uint8_t*) malloc(outPortDataLen);
                        assert(nodeData->outPortDirtyFlags);
                        nodeData->outPortDataLen = outPortDataLen;
#endif
#if 0
                        APX_LOG_INFO("[APX_NODE_MANAGER]%s Server opening client file %s[%d,%d]", debugInfoStr, fileName, outDataFile->fileInfo.address, outDataFile->fileInfo.length);
#endif
                        apx_nodeData_setNodeInfo(nodeData, nodeInfo);
#if 0
                        apx_fileManager_sendFileOpen(fileManager, outDataFile->fileInfo.address);
#endif
                     }
                  }
               }
               else
               {
                  APX_LOG_WARNING("[APX_NODE_MANAGER] '%s': no file found", fileName);
               }
            }
#if 0
            if (inPortDataLen > 0)
            {
               //create local inPortData file
               apx_file2_t *inDataFile;
               bool result;
               strcpy(fileName,apxNode->name);
               p=fileName+strlen(fileName);
               strcpy(p,".in");
               
               nodeData->inPortDataBuf = (uint8_t*) malloc(inPortDataLen);
               assert(nodeData->inPortDataBuf);
               nodeData->inPortDirtyFlags = (uint8_t*) malloc(inPortDataLen);
               assert(nodeData->inPortDirtyFlags);
               result = apx_nodeManager_createInitData(apxNode, nodeData->inPortDataBuf, inPortDataLen);
               if (result == false)
               {
                  APX_LOG_ERROR("[APX_NODE_MANAGER] Failed to create init data for node %s", apx_node_getName(apxNode));
               }
               nodeData->inPortDataLen = inPortDataLen;
               inDataFile = apx_file_newLocalInPortDataFile(nodeData);
               if (inDataFile != 0)
               {
#if 0
                 apx_fileManager_attachLocalPortDataFile(fileManager, inDataFile);
                  APX_LOG_INFO("[APX_NODE_MANAGER]%s Server created file %s[%d,%d]", debugInfoStr, fileName, inDataFile->fileInfo.address, inDataFile->fileInfo.length);
#endif
               }
               else
               {
#if 0
                  APX_LOG_ERROR("[APX_NODE_MANAGER]%s Server failed to create local file '%s'", debugInfoStr, fileName);
#endif
               }
#endif
            }
            //router is set, attach the newly create nodeInfo to the router
            if (self->router != 0)
            {
               apx_router_attachNodeInfo(self->router, nodeInfo);
            }
            //for all connected require ports copy data from the provide port into our newly create inDataFile buffer
            apx_nodeInfo_copyInitDataFromProvideConnectors(nodeInfo);
         }
      }
      apx_parser_clearNodes(&self->parser);
   }
#endif
}

/**
 * adds nodeData to localNodeDataMap
 */
static void apx_nodeManager_setLocalNodeData(apx_nodeManager_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      adt_hash_set(&self->localNodeDataMap, nodeData->name, 0, nodeData);
   }
}

/**
 * applies the portTriggerFunction
 */
static void apx_nodeManager_executePortTriggerFunction(const apx_dataTriggerFunction_t *triggerFunction, const apx_file2_t *file)
{
#if 0
   if( (triggerFunction != 0) && (file != 0) )
   {
      if (file->fileType == APX_OUTDATA_FILE)
      {
         uint8_t *dataBuf = (uint8_t*) malloc(triggerFunction->dataLength);
         if (dataBuf != 0)
         {
            int8_t result = apx_nodeData_readOutPortData(file->nodeData, dataBuf, triggerFunction->srcOffset, triggerFunction->dataLength);
            if (result == 0)
            {
               int32_t i;
               int32_t end = adt_ary_length(&triggerFunction->writeInfoList);
               for(i=0;i<end;i++)
               {
                  apx_dataWriteInfo_t *writeInfo = (apx_dataWriteInfo_t*) adt_ary_value(&triggerFunction->writeInfoList, i);
                  apx_nodeInfo_t *targetNodeInfo = writeInfo->requesterNodeInfo;
                  if( targetNodeInfo->nodeData != 0)
                  {
                     apx_nodeData_t *targetNodeData = targetNodeInfo->nodeData;
                     if( (targetNodeData->inPortDataFile != 0) && (targetNodeData->fileManager != 0) )
                     {
#if 0
                        apx_fileManager_triggerFileWriteCmdEvent(targetNodeData->fileManager, targetNodeData->inPortDataFile, dataBuf, writeInfo->destOffset, triggerFunction->dataLength);
#endif
                     }
                  }
               }
            }
            free(dataBuf);
         }
      }
   }
#endif
}


static void apx_nodeManager_removeRemoteNodeData(apx_nodeManager_t *self, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      void **tmp = adt_hash_remove(&self->remoteNodeDataMap, nodeData->name, 0);
      assert(tmp != 0);
   }
}

static void apx_nodeManager_removeNodeInfo(apx_nodeManager_t *self, apx_nodeInfo_t *nodeInfo)
{
   if ( (self != 0) && (nodeInfo != 0) )
   {
      void **tmp = adt_hash_remove(&self->nodeInfoMap, nodeInfo->node->name, 0);
      assert(tmp != 0);
   }
}

static bool apx_nodeManager_createInitData(apx_node_t *node, uint8_t *buf, int32_t bufLen)
{
   if ( (node != 0) && (buf != 0) && (bufLen > 0))
   {
      uint8_t *pNext = buf;
      uint8_t *pEnd = buf+bufLen;
      int32_t i;
      int32_t numRequirePorts;
      adt_bytearray_t *portData;
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
      return true;
   }
   return false;
}

static void apx_nodeManager_attachLocalNodesToFileManager(apx_nodeManager_t *self, apx_fileManager_t *fileManager)
{
   adt_hash_iter_init(&self->localNodeDataMap);
   void **ppVal;
   do
   {
      const char *key;
      uint32_t keyLen;
      ppVal = adt_hash_iter_next(&self->localNodeDataMap, &key, &keyLen);
      if (ppVal != 0 )
      {
         apx_nodeData_t *nodeData = *ppVal;
         apx_nodeManager_attachLocalNodeToFileManager(nodeData, fileManager);
      }
   } while(ppVal != 0);
}

static void apx_nodeManager_attachLocalNodeToFileManager(apx_nodeData_t *nodeData, apx_fileManager_t *fileManager)
{
#if 0
   if (nodeData->definitionDataLen > 0)
   {
      apx_file2_t *definitionFile = apx_file_newLocalDefinitionFile(nodeData);
      if (definitionFile != 0)
      {
         apx_fileManager_attachLocalFile(fileManager, definitionFile);
      }
   }
   if (nodeData->outPortDataLen > 0)
   {
      apx_file2_t *outPortDataFile = apx_file_newLocalOutPortDataFile(nodeData);
      if (outPortDataFile != 0)
      {
         apx_nodeData_setOutPortDataFile(nodeData, outPortDataFile);
         apx_fileManager_attachLocalFile(fileManager, outPortDataFile);
      }
   }
#endif
}

static void apx_nodeManager_registerEventListenerToFileManager(apx_nodeManager_t *self, apx_fileManager_t *fileManager)
{
#if 0
   apx_fileManagerEventListener_t listener;
   memset(&listener, 0, sizeof(listener));
   listener.arg = self;
   listener.fileCreate = apx_nodeManager_remoteFileCreated;
   listener.fileRevoke = apx_nodeManager_remoteFileRevoked;
   listener.fileWrite = apx_nodeManager_remoteFileWritten;
   apx_fileManager_registerEventListener(fileManager, &listener);
#endif
}


static void apx_nodeManager_remoteFileCreated(void *arg, struct apx_fileManager_tag *fileManager, apx_file2_t *remoteFile)
{
#if 0
   apx_nodeManager_t *self = (apx_nodeManager_t *) arg;
   if ( (self != 0) && (fileManager != 0) && (remoteFile != 0) && (remoteFile->isRemoteFile == true))
   {
      if ( remoteFile->fileType == APX_DEFINITION_FILE )
      {
         char *basename = apx_file_basename(remoteFile);
         if (basename != 0)
         {
            apx_nodeData_t *nodeData;
            //this is potentially a new node, check if it exists already
            MUTEX_LOCK(self->mutex);
            nodeData = apx_nodeManager_findNodeData(self, basename);
            MUTEX_UNLOCK(self->mutex);
            if (nodeData == 0)
            {
               if (fileManager->mode == APX_FILEMANAGER_SERVER_MODE)
               {
                  //create new nodeData structure and initiate download of file
                  nodeData = apx_nodeData_newRemote(basename, false); //setting weakref to false will force apx_nodeData_delete to delete all buffers we created here
                  if (nodeData != 0)
                  {
                     nodeData->definitionDataBuf = (uint8_t*) malloc(remoteFile->fileInfo.length);
                     if (nodeData->definitionDataBuf==0)
                     {
                        APX_LOG_ERROR("[APX_NODE_MANAGER] out of memory when attempting to create definitionDataBuf of length %d for node %s", (int) remoteFile->fileInfo.length, basename);
                        free(basename);
                        apx_nodeData_delete(nodeData);
                        return;
                     }
                     else
                     {
                        nodeData->definitionDataLen = remoteFile->fileInfo.length;
                        MUTEX_LOCK(self->mutex);
                        adt_hash_set(&self->remoteNodeDataMap, basename, 0, nodeData);
                        MUTEX_UNLOCK(self->mutex);
                        //now that memory has been allocated, send request to open the file (triggering file transfer)
#if 0
                        apx_fileManager_sendFileOpen(fileManager, remoteFile->fileInfo.address);
#endif
                        //the following line binds our new nodeData object to the apx_file2_t structure
                        remoteFile->nodeData=nodeData;
                     }
                  }
               }
               else
               {
                  //client mode
               }
            }
            else
            {
               APX_LOG_ERROR("[APX_NODE_MANAGER] node already exists: %s", basename);
            }
            free(basename);
         }
      }
      else if ( (remoteFile->fileType == APX_INDATA_FILE) )
      {
         char *basename = apx_file_basename(remoteFile);
         if (basename != 0)
         {
            apx_nodeData_t *nodeData;
            //this is potentially a new node, check if it exists already
            MUTEX_LOCK(self->mutex);
            nodeData = apx_nodeManager_findNodeData(self, basename);
            MUTEX_UNLOCK(self->mutex);
            if ( nodeData != 0 )
            {
               if (nodeData->inPortDataLen != remoteFile->fileInfo.length)
               {
#if 0
                  APX_LOG_ERROR("[APX_NODE_MANAGER(%s)] file %s has length %d, expected %d\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name, remoteFile->fileInfo.length, nodeData->inPortDataLen);
#endif
               }
               else
               {
                  if ( nodeData->inPortDataBuf == 0)
                  {
#if 0
                     APX_LOG_ERROR("[APX_NODE_MANAGER(%s)] cannot open file %s, inPortDataBuf is NULL\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name);
#endif
                  }
                  else
                  {
                     remoteFile->nodeData=nodeData;
#if 0
                     apx_fileManager_sendFileOpen(fileManager, remoteFile->fileInfo.address);
#endif
                  }
               }
            }
            free(basename);
         }
      }
      else if (remoteFile->fileType == APX_USER_DATA_FILE)
      {
         if(self->debugMode >= APX_DEBUG_2_LOW)
         {
#if 0
            APX_LOG_INFO("[APX_NODE_MANAGER(%s)] Unsupported file: %s\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name);
#endif
         }
      }
      else
      {

      }
   }
#endif
}


/**
 * this is caled by fileManager when a file has been removed
 */
static void apx_nodeManager_remoteFileRevoked(void *arg, struct apx_fileManager_tag *fileManager, apx_file2_t *remotefile)
{
   //printf("apx_nodeManager_remotefileRemoved\n");
}

/**
 * this is called by fileManager when a file has been written to
 */
static void apx_nodeManager_remoteFileWritten(void *arg, struct apx_fileManager_tag *fileManager, const apx_file2_t *remoteFile, uint32_t offset, int32_t length)
{
#if 0
   apx_nodeManager_t *self = (apx_nodeManager_t *) arg;
   if ( (self != 0) && (remoteFile != 0) )
   {
      if (remoteFile->fileType == APX_DEFINITION_FILE)
      {
         MUTEX_LOCK(self->mutex);
         apx_nodeManager_createNode(self, remoteFile->nodeData->definitionDataBuf, remoteFile->nodeData->definitionDataLen, fileManager);
         MUTEX_UNLOCK(self->mutex);
      }
      else
      {
         uint32_t endOffset = offset + length;
         if ( (offset == 0) && (length == (int32_t) remoteFile->fileInfo.length) )
         {
            //printf("[APX_NODE_MANAGER(%s)] file received name=%s, len=%u\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name, length);
         }
         else
         {
            //printf("[APX_NODE_MANAGER(%s)] file updated name=%s, offset=%d, len=%u\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name, offset, length);
         }
         if (remoteFile->fileType == APX_OUTDATA_FILE)
         {
            apx_nodeInfo_t *nodeInfo = remoteFile->nodeData->nodeInfo;
            assert(nodeInfo != 0);
            while (offset < endOffset)
            {
               apx_dataTriggerFunction_t *triggerFunction;
               triggerFunction = apx_nodeInfo_getTriggerFunction(nodeInfo, offset);
               if (triggerFunction != 0)
               {
                  apx_nodeManager_executePortTriggerFunction(triggerFunction, remoteFile);
                  offset = triggerFunction->srcOffset + triggerFunction->dataLength;
               }
               else
               {
                  //fprintf(stderr, "APX_NODE_MANAGER(%s)] no trigger function found for file %s at offset %d\n", apx_fileManager_modeString(fileManager), remoteFile->fileInfo.name, offset);
                  offset++;
               }
            }
         }
      }
   }
#endif
}

