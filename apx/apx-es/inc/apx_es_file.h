#ifndef APX_FILE_H
#define APX_FILE_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "apx_types.h"
#include "apx_nodeData.h"
#include "rmf.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

typedef struct apx_es_file_tag
{
   bool isRemoteFile;
   bool isOpen;
   uint8_t fileType;
   apx_nodeData_t *nodeData;
   rmf_fileInfo_t fileInfo;
} apx_es_file_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_es_file_createLocalFileFromNodeData(apx_es_file_t *self, uint8_t fileType, apx_nodeData_t *nodeData);
int8_t apx_es_file_create(apx_es_file_t *self, uint8_t fileType, const rmf_fileInfo_t *fileInfo);
char *apx_es_file_basename(const apx_es_file_t *self);
void apx_es_file_open(apx_es_file_t *self);
void apx_es_file_close(apx_es_file_t *self);
int8_t apx_es_file_read(apx_es_file_t *self, uint8_t *pDest, uint32_t offset, uint32_t length);
int8_t apx_es_file_write(apx_es_file_t *self, const uint8_t *pSrc, uint32_t offset, uint32_t length);

#endif //APX_FILE_H

