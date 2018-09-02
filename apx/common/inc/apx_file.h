#ifndef APX_FILE_H
#define APX_FILE_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#if defined(_MSC_PLATFORM_TOOLSET) && (_MSC_PLATFORM_TOOLSET<=110)
#include "msc_bool.h"
#else
#include <stdbool.h>
#endif
#include "apx_nodeData.h"
#include "rmf.h"
#ifndef APX_EMBEDDED
#include "apx_eventListener.h"
#endif
#include "apx_types.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////




typedef struct apx_file_tag
{
   bool isRemoteFile;
   bool isOpen;
   uint8_t fileType;
   apx_nodeData_t *nodeData;
   rmf_fileInfo_t fileInfo;
} apx_file_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_file_createLocalFileFromNodeData(apx_file_t *self, uint8_t fileType, apx_nodeData_t *nodeData);
int8_t apx_file_create(apx_file_t *self, uint8_t fileType, const rmf_fileInfo_t *fileInfo);
#ifndef APX_EMBEDDED
void apx_file_destroy(apx_file_t *self);
apx_file_t *apx_file_newLocalFileFromNodeData(uint8_t fileType, apx_nodeData_t *nodeData);
apx_file_t *apx_file_newLocalDefinitionFile(apx_nodeData_t *nodeData);
apx_file_t *apx_file_newLocalOutPortDataFile(apx_nodeData_t *nodeData);
apx_file_t *apx_file_newLocalInPortDataFile(apx_nodeData_t *nodeData);
apx_file_t *apx_file_new(uint8_t fileType, const rmf_fileInfo_t *fileInfo);
void apx_file_delete(apx_file_t *self);
void apx_file_vdelete(void *arg);
#endif
char *apx_file_basename(const apx_file_t *self);
void apx_file_open(apx_file_t *self);
void apx_file_close(apx_file_t *self);
int8_t apx_file_read(apx_file_t *self, uint8_t *pDest, uint32_t offset, uint32_t length);
int8_t apx_file_write(apx_file_t *self, const uint8_t *pSrc, uint32_t offset, uint32_t length);

#endif //APX_FILE_H

