//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "apx_es_file.h"
#include "apx_logging.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static uint8_t apx_es_file_deriveFileType(apx_es_file_t *self);

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int8_t apx_es_file_createLocalFileFromNodeData(apx_es_file_t *self, uint8_t fileType, apx_nodeData_t *nodeData)
{
   if ( (self != 0) && (nodeData != 0) )
   {
      size_t len;
      char name[RMF_MAX_FILE_NAME+1];
      self->fileType=fileType;
      self->nodeData=nodeData;
      self->isRemoteFile = false;
      self->isOpen = false;
      len = strlen(self->nodeData->name);

      if (len+APX_MAX_FILE_EXT_LEN <= RMF_MAX_FILE_NAME)
      {
         const char *ext=0;
         uint32_t filelen = 0;

         memcpy(name, self->nodeData->name, len);
         switch(fileType)
         {
         case APX_OUTDATA_FILE:
            ext = APX_OUTDATA_FILE_EXT;
            filelen = nodeData->outPortDataLen;
            break;
         case APX_INDATA_FILE:
            ext = APX_INDATA_FILE_EXT;
            filelen = nodeData->inPortDataLen;
            break;
         case APX_DEFINITION_FILE:
            ext = APX_DEFINITION_FILE_EXT;
            filelen = nodeData->definitionDataLen;
            break;
         default:
            errno = EINVAL;
            return -1;
         }
         strcpy(name+len, ext);
         rmf_fileInfo_create(&self->fileInfo, name, RMF_INVALID_ADDRESS, filelen, RMF_FILE_TYPE_FIXED);
         return 0;
      }
   }
   errno = EINVAL;
   return -1;
}

int8_t apx_es_file_create(apx_es_file_t *self, uint8_t fileType, const rmf_fileInfo_t *fileInfo)
{
   if ( (self != 0) && (fileType<=APX_EVENT_FILE) && (fileInfo != 0) )
   {
      int8_t result;
      self->isRemoteFile = true;
      self->nodeData = 0;
      self->isOpen = false;

      result = rmf_fileInfo_create(&self->fileInfo, fileInfo->name, fileInfo->address, fileInfo->length, fileInfo->fileType);
      if (result == 0)
      {
         rmf_fileInfo_setDigestData(&self->fileInfo, fileInfo->digestType, fileInfo->digestData, 0);
         if (fileType == APX_UNKNOWN_FILE)
         {
            self->fileType = apx_es_file_deriveFileType(self);
         }
         else
         {
            self->fileType = fileType;
         }
      }
      return result;
   }
   errno = EINVAL;
   return -1;
}

void apx_es_file_destroy(apx_es_file_t *self)
{
   if (self != 0)
   {
      rmf_fileInfo_destroy(&self->fileInfo);
   }
}

void apx_es_file_open(apx_es_file_t *self)
{
   if (self != 0)
   {
      self->isOpen = true;
   }
}

void apx_es_file_close(apx_es_file_t *self)
{
   if (self != 0)
   {
      self->isOpen = false;
   }
}

int8_t apx_es_file_read(apx_es_file_t *self, uint8_t *pDest, uint32_t offset, uint32_t length)
{
   if ( (self != 0) && (pDest != 0) && (length > 0) )
   {
      int8_t result;
      switch(self->fileType)
      {
      case APX_UNKNOWN_FILE:
         result = 0;
         break;
      case APX_OUTDATA_FILE:
         result = apx_nodeData_readOutPortData(self->nodeData, pDest, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_readOutPortData failed");
         }
         break;
      case APX_INDATA_FILE:
         result = apx_nodeData_readInPortData(self->nodeData, pDest, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_writeInData failed");
         }
         break;
      case APX_DEFINITION_FILE:
         result = apx_nodeData_readDefinitionData(self->nodeData, pDest, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_readDefinitionData failed");
         }
         break;
      default:
         result=-1;
         break;
      }
      return result;
   }
   return -1;
}

int8_t apx_es_file_write(apx_es_file_t *self, const uint8_t *pSrc, uint32_t offset, uint32_t length)
{

   if ( (self != 0) && (pSrc != 0) && (self->nodeData != 0) )
   {
      int8_t result;
      switch(self->fileType)
      {
      case APX_DEFINITION_FILE:
         result = apx_nodeData_writeDefinitionData(self->nodeData, pSrc, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_writeDefinitionData failed with %d", result);
         }
         break;
      case APX_INDATA_FILE:
         result = apx_nodeData_writeInPortData(self->nodeData, pSrc, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_writeInPortData failed with %d", result);            
         }
         else
         {
            apx_nodeData_triggerInPortDataWritten(self->nodeData, offset, length);
         }
         break;
      case APX_OUTDATA_FILE:
         result = apx_nodeData_writeOutPortData(self->nodeData, pSrc, offset, length);
         if (result != 0)
         {
            APX_LOG_ERROR("[APX_FILE] apx_nodeData_writeOutPortData failed with %d", result);            
         }
         break;
      default:
         result=-1;
         break;
      }
      return result;
   }
   return -1;
}

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
/**
 * gets file extension from file name and uses that to set fileType property
 */
static uint8_t apx_es_file_deriveFileType(apx_es_file_t *self)
{
   if (self != 0)
   {
      size_t len = strlen(self->fileInfo.name);
      if (len > 0 ) //4 characters means that there is room for file extension
      {
         char *pBegin = self->fileInfo.name;
         char *p = self->fileInfo.name+len-1;
         //search in string backwards to find a '.' character
         while(p>=pBegin)
         {
            if (*p == '.')
            {
               if ( (strcmp(p, APX_DEFINITION_FILE_EXT)==0) )
               {
                  return APX_DEFINITION_FILE;
               }
               else if ( (strcmp(p, APX_INDATA_FILE_EXT)==0) )
               {
                  return APX_INDATA_FILE;
               }
               else if ( (strcmp(p, APX_OUTDATA_FILE_EXT)==0) )
               {
                  return APX_OUTDATA_FILE;
               }
               else
               {
                  return APX_USER_DATA_FILE;
               }
            }
            --p;
         }
         return APX_USER_DATA_FILE;
      }
   }
   return APX_UNKNOWN_FILE;
}


