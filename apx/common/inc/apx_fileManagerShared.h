#ifndef APX_FILE_MANAGER_SHARED_H
#define APX_FILE_MANAGER_SHARED_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "apx_allocator.h"
#include "apx_fileMap.h"
#include "apx_fileManagerDefs.h"
#include "rmf.h"

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
//forward declarations
struct apx_file_tag;


typedef struct apx_fileManagerShared_tag
{
   void *arg;
   apx_allocator_t allocator;
   uint32_t fmid; //a.k.a channel ID
   void (*fileOpenRequestedByRemote)(void *arg, const rmf_cmdOpenFile_t *cmdOpenFile);
   void (*fileCreatedByRemote)(void *arg, const struct apx_file_tag *pFile);
}apx_fileManagerShared_t;


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int8_t apx_fileManagerShared_create(apx_fileManagerShared_t *self, uint32_t fmid);
void apx_fileManagerShared_destroy(apx_fileManagerShared_t *self);
uint8_t *apx_fileManagerShared_alloc(apx_fileManagerShared_t *self, size_t size);
void apx_fileManagerShared_free(apx_fileManagerShared_t *self, uint8_t *ptr, size_t size);


#endif //APX_FILE_MANAGER_SHARED_H
