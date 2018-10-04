/**
 * file: apx_nodeData.h
 * description: apx_nodeData is the client version of an apx node. The server side uses the more complex class called apx_nodeInfo.
 */
#ifndef APX_NODE_DATA_H
#define APX_NODE_DATA_H
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_types.h"
#include "apx_error.h"
#include "apx_eventListener.h"
#include <stdint.h>
#ifndef APX_EMBEDDED
#  ifndef _WIN32
     //Linux-based system
#    include <pthread.h>
#  else
     //Windows-based system
#    include <Windows.h>
#  endif
#  include "osmacro.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_file2_tag;
#ifdef APX_EMBEDDED
struct apx_es_fileManager_tag;
#else
struct apx_fileManager_tag;
struct apx_nodeInfo_tag;
#endif

//forward declaration
struct apx_nodeData_tag;
struct apx_node_tag;

/**
 * function table of event handlers that apx_nodeData_t can call when events are triggererd
 */

typedef void (apx_nodeData_nodeCbkFunc)(uint32_t offset, uint32_t len);

typedef struct apx_nodeData_tag
{
   bool isRemote; //true if this is a remote nodeData structure. Default: false
   bool isWeakref; //when true all pointers in this object is owned by some other part of the program. if false then all pointers are created/freed by this class.
   const char *name; //only used when node is code-generated
   uint8_t *inPortDataBuf;
   uint8_t *outPortDataBuf;
   uint32_t inPortDataLen;
   uint32_t outPortDataLen;
   uint8_t *definitionDataBuf;
   uint32_t definitionDataLen;
   uint8_t *inPortDirtyFlags;
   uint8_t *outPortDirtyFlags;
   uint32_t definitionStartOffset; //used when more_bit=true during large writes
   uint32_t outPortDataStartOffset; //used when more_bit=true during large writes
   uint32_t inPortDataStartOffset; //used when more_bit=true during large writes
   apx_nodeData_nodeCbkFunc *apxNodeWriteCbk; //This is used to trigger callbacks in static ApxNode files generated by Python
   apx_nodeDataEventListener_t eventListener; //only support one a single event listener (in addition to the Python-generated callback trigger)
   uint8_t checksumType;
   uint8_t checksumData[APX_CHECKSUMLEN_SHA256];
#ifdef APX_EMBEDDED
   //used for implementations that has no underlying operating system or runs an RTOS
   struct apx_es_fileManager_tag *fileManager;
#else
   //used for Windows/Linux implementations
   struct apx_fileManager_tag *fileManager;
   SPINLOCK_T inPortDataLock;
   SPINLOCK_T outPortDataLock;
   SPINLOCK_T definitionDataLock;
   SPINLOCK_T internalLock;
   struct apx_node_tag *node;
#endif
   struct apx_file2_tag *definitionFile;
   struct apx_file2_tag *outPortDataFile;
   struct apx_file2_tag *inPortDataFile;
   struct apx_nodeInfo_tag *nodeInfo;
} apx_nodeData_t;


//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void apx_nodeData_create(apx_nodeData_t *self, const char *name, uint8_t *definitionBuf, uint32_t definitionDataLen, uint8_t *inPortDataBuf, uint8_t *inPortDirtyFlags, uint32_t inPortDataLen, uint8_t *outPortDataBuf, uint8_t *outPortDirtyFlags, uint32_t outPortDataLen);
void apx_nodeData_destroy(apx_nodeData_t *self);
#ifndef APX_EMBEDDED
apx_nodeData_t *apx_nodeData_new(uint32_t definitionDataLen);
void apx_nodeData_delete(apx_nodeData_t *self);
void apx_nodeData_vdelete(void *arg);
#endif
apx_error_t apx_nodeData_setChecksumData(apx_nodeData_t *self, uint8_t checksumType, uint8_t *checksumData);
bool apx_nodeData_isOutPortDataOpen(apx_nodeData_t *self);
void apx_nodeData_setEventListener(apx_nodeData_t *self, apx_nodeDataEventListener_t *eventListener);
void apx_nodeData_setApxNodeCallback(apx_nodeData_t *self, apx_nodeData_nodeCbkFunc* cbk);
apx_error_t apx_nodeData_readDefinitionData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len);
apx_error_t apx_nodeData_readOutPortData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len);
apx_error_t apx_nodeData_readInPortData(apx_nodeData_t *self, uint8_t *dest, uint32_t offset, uint32_t len);
void apx_nodeData_lockOutPortData(apx_nodeData_t *self);
void apx_nodeData_unlockOutPortData(apx_nodeData_t *self);
void apx_nodeData_lockInPortData(apx_nodeData_t *self);
void apx_nodeData_unlockInPortData(apx_nodeData_t *self);
void apx_nodeData_outPortDataNotify(apx_nodeData_t *self, uint32_t offset, uint32_t len);
apx_error_t apx_nodeData_writeInPortData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len);
apx_error_t apx_nodeData_writeOutPortData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len);
apx_error_t apx_nodeData_writeDefinitionData(apx_nodeData_t *self, const uint8_t *src, uint32_t offset, uint32_t len);
void apx_nodeData_setInPortDataFile(apx_nodeData_t *self, struct apx_file2_tag *file);
void apx_nodeData_setOutPortDataFile(apx_nodeData_t *self, struct apx_file2_tag *file);
void apx_nodeData_setDefinitionFile(apx_nodeData_t *self, struct apx_file2_tag *file);
#ifdef APX_EMBEDDED
void apx_nodeData_setFileManager(apx_nodeData_t *self, struct apx_es_fileManager_tag *fileManager);
#else
void apx_nodeData_setFileManager(apx_nodeData_t *self, struct apx_fileManager_tag *fileManager);
apx_error_t apx_nodeData_createPortDataBuffers(apx_nodeData_t *self);
void apx_nodeData_setNodeInfo(apx_nodeData_t *self, struct apx_nodeInfo_tag *nodeInfo);
void apx_nodeData_setNode(apx_nodeData_t *self, struct apx_node_tag *node);
const char *apx_nodeData_getName(apx_nodeData_t *self);

#ifdef UNIT_TEST
struct apx_file2_tag *apx_nodeData_newLocalDefinitionFile(apx_nodeData_t *self);
struct apx_file2_tag *apx_nodeData_newLocalOutPortDataFile(apx_nodeData_t *self);
#endif //UNIT_TEST
#endif //APX_EMBEDDED
#endif //APX_NODE_DATA_H
