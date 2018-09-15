#include <errno.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "bstr.h"
#include "apx_node.h"
#include "apx_nodeInfo.h"
#include "apx_logging.h"
#include "apx_error.h"
#include "apx_types.h"

//#include "pack.h"
#include "adt_hash.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

#define ERROR_STR_MAX 128
/**************** Private Function Declarations *******************/
static void apx_parser_attributeParseError(apx_port_t *port, int32_t lastError);

/**************** Private Variable Declarations *******************/


/****************** Public Function Definitions *******************/
//constructor/destructor
apx_node_t *apx_node_new(const char *name){
   apx_node_t *self = (apx_node_t*) malloc(sizeof(apx_node_t));
   if(self != 0){
      apx_node_create(self,name);
   }
   else{
      errno = ENOMEM;
   }
   return self;
}

void apx_node_delete(apx_node_t *self){
   if(self != 0){
      apx_node_destroy(self);
      free(self);
   }
}

void apx_node_vdelete(void *arg)
{
   apx_node_delete((apx_node_t*) arg);
}

void apx_node_create(apx_node_t *self,const char *name){
   if(self != 0){
      self->name = 0;
      adt_ary_create(&self->datatypeList,apx_datatype_vdelete);
      adt_ary_create(&self->requirePortList,apx_port_vdelete);
      adt_ary_create(&self->providePortList,apx_port_vdelete);
      apx_attributeParser_create(&self->attributeParser);
      apx_node_setName(self,name);
      //self->lastPortError=APX_NO_ERROR;
      //self->lastPortId=-1;
      //self->lastPortType=-1;
      self->nodeInfo=(apx_nodeInfo_t*) 0;
      self->isFinalized = false;
   }
}

void apx_node_destroy(apx_node_t *self){
   if(self != 0){
      adt_ary_destroy(&self->datatypeList);
      adt_ary_destroy(&self->providePortList);
      adt_ary_destroy(&self->requirePortList);
      apx_attributeParser_destroy(&self->attributeParser);
      if(self->name != 0){
         free(self->name);
      }
   }
}

//node functions
void apx_node_setName(apx_node_t *self, const char *name){
   if( (self != 0) ){
      if(self->name != 0){
         free(self->name);
      }
      if(name != 0){
         self->name = STRDUP(name);
      }
      else{
         self->name = 0;
      }
   }
}

const char *apx_node_getName(apx_node_t *self)
{
   if (self != 0)
   {
      return self->name;
   }
   return (const char*) 0;
}

//datatype functions
apx_datatype_t *apx_node_createDataType(apx_node_t *self, const char* name, const char *dsg, const char *attr)
{
   apx_datatype_t *datatype=0;
   if (self != 0)
   {
     datatype = apx_datatype_new(name,dsg,attr, 0);
     if (datatype != 0)
     {
        adt_ary_push(&self->datatypeList,datatype);
     }
   }
   return datatype;
}

//port functions
apx_port_t *apx_node_createRequirePort(apx_node_t *self, const char* name, const char *dsg, const char *attr)
{
   apx_port_t *port=0;
   if (self != 0)
   {
     port = apx_requirePort_new(name,dsg,attr);
     if (port != 0)
     {
        int32_t portIndex = adt_ary_length(&self->requirePortList);
        if ( port->portAttributes != 0 )
        {
           bool result = apx_attributeParser_parseObject(&self->attributeParser, port->portAttributes);
           if (result == false)
           {
              int32_t lastError;
              lastError = apx_attributeParser_getLastError(&self->attributeParser, 0);
              apx_parser_attributeParseError(port, lastError);
              apx_port_delete(port);
              return 0;
           }
        }
        apx_port_setPortIndex(port,portIndex);
        adt_ary_push(&self->requirePortList,port);
     }
   }
   return port;
}

apx_port_t *apx_node_createProvidePort(apx_node_t *self, const char* name, const char *dsg, const char *attr)
{
   apx_port_t *port=0;
   if (self != 0)
   {
     port = apx_providePort_new(name,dsg,attr);
     if (port != 0)
     {
        int32_t portIndex = adt_ary_length(&self->providePortList);
        if ( port->portAttributes != 0 )
        {
           bool result = apx_attributeParser_parseObject(&self->attributeParser, port->portAttributes);
           if (result == false)
           {
              int32_t lastError;
              lastError = apx_attributeParser_getLastError(&self->attributeParser, 0);
              apx_parser_attributeParseError(port, lastError);
              apx_port_delete(port);
              return 0;
           }
        }
        apx_port_setPortIndex(port,portIndex);
        adt_ary_push(&self->providePortList,port);
     }
   }
   return port;
}

/**
 * return 0 on success, -1 on error
 */
apx_error_t apx_node_finalize(apx_node_t *self, int32_t *lineNumber)
{
   apx_error_t retval = APX_INVALID_ARGUMENT_ERROR;
   if (self != 0)
   {
      if (self->isFinalized == true)
      {
         retval = APX_NO_ERROR;
      }
      else
      {
         int32_t i;
         int32_t dataTypeLen;
         int32_t providePortLen;
         int32_t requirePortLen;
         adt_hash_t typeMap;

         adt_hash_create(&typeMap, NULL);
         dataTypeLen = adt_ary_length(&self->datatypeList);
         for(i=0;i<dataTypeLen;i++)
         {
            apx_datatype_t *datatype = (apx_datatype_t*) adt_ary_value(&self->datatypeList, i);
            if ( (datatype->name != 0) && (strlen(datatype->name)>0) )
            {
               adt_hash_set(&typeMap, datatype->name, 0, datatype);
            }
         }
         providePortLen = adt_ary_length(&self->providePortList);
         for(i=0;i<providePortLen;i++)
         {
            apx_error_t result;
            apx_port_t *port = (apx_port_t*) adt_ary_value(&self->providePortList, i);
            result = apx_port_resolveTypes(port, &self->datatypeList, &typeMap);
            if (result != APX_NO_ERROR)
            {
               if (lineNumber != 0) *lineNumber = port->lineNumber;
               retval = result;
               break;
            }
            result = apx_port_updateDerivedPortSignature(port);
            {
               if (result != APX_NO_ERROR)
               {
                  if (lineNumber != 0) *lineNumber = port->lineNumber;
                  retval = result;
                  break;
               }
            }
         }
         requirePortLen = adt_ary_length(&self->requirePortList);
         for(i=0;i<requirePortLen;i++)
         {
            apx_error_t result;
            apx_port_t *port = (apx_port_t*) adt_ary_value(&self->requirePortList, i);
            result = apx_port_resolveTypes(port, &self->datatypeList, &typeMap);
            if (result != APX_NO_ERROR)
            {
               if (lineNumber != 0) *lineNumber = port->lineNumber;
               retval = result;
               break;
            }
            result = apx_port_updateDerivedPortSignature(port);
            {
               if (result != APX_NO_ERROR)
               {
                  if (lineNumber != 0) *lineNumber = port->lineNumber;
                  retval = result;
                  break;
               }
            }
         }
         adt_hash_destroy(&typeMap);
         self->isFinalized = true;
         retval = APX_NO_ERROR;
      }
   }
   return retval;
}

apx_port_t *apx_node_getRequirePort(apx_node_t *self, int32_t portIndex)
{
   if (self != 0)
   {
      return (apx_port_t*) *adt_ary_get(&self->requirePortList,portIndex);
   }
   return (apx_port_t*) 0;
}

apx_port_t *apx_node_getProvidePort(apx_node_t *self, int32_t portIndex)
{
   if (self != 0)
   {
      return (apx_port_t*) *adt_ary_get(&self->providePortList,portIndex);
   }
   return (apx_port_t*) 0;
}

int32_t apx_node_getNumRequirePorts(apx_node_t *self)
{
   if ( self != 0 )
   {
      return adt_ary_length(&self->requirePortList);
   }
   errno=EINVAL; //only set errno if self==0
   return -1;
}

int32_t apx_node_getNumProvidePorts(apx_node_t *self)
{
   if ( self != 0 )
   {
      return adt_ary_length(&self->providePortList);
   }
   errno=EINVAL; //only set errno if self==0
   return -1;
}

adt_bytearray_t *apx_node_createPortInitData(apx_node_t *self, apx_port_t *port)
{
   if ( (self != 0) && (port != 0) )
   {
      int32_t result;
      adt_bytearray_t *initData = adt_bytearray_new(0);
      result = apx_node_fillPortInitData(self, port, initData);
      if (result != 0)
      {
         adt_bytearray_delete(initData);
         return 0;
      }
      return initData;
   }
   errno=EINVAL;
   return 0;
}


/***************** Private Function Definitions *******************/

static void apx_parser_attributeParseError(apx_port_t *port, int32_t lastError)
{
   char errorStr[ERROR_STR_MAX+1];
   uint32_t remain = ERROR_STR_MAX;
   uint32_t errorStrLen=0;
   uint32_t attrLen = strlen(port->portAttributes->rawValue);
   switch(lastError)
   {
   case APX_PARSE_ERROR:
      errorStrLen = sprintf(errorStr, "Failed to parse port attribute string: ");
      break;
   default:
      return;
   }
   remain -= errorStrLen;
   if (remain >= attrLen)
   {
      strcpy(&errorStr[errorStrLen], port->portAttributes->rawValue);
   }
   else
   {
      //truncate the port attribute string adding "..." at the end
      uint32_t bytesToCopy = remain-3;
      strncpy(&errorStr[errorStrLen], port->portAttributes->rawValue, bytesToCopy);
      strcpy(&errorStr[errorStrLen+bytesToCopy], "...");
   }
   APX_LOG_ERROR("%s", errorStr);
}

/**
 * returns zero on success, non-zero on error
 */
int32_t apx_node_fillPortInitData(apx_node_t *self, apx_port_t *port, adt_bytearray_t *output)
{
   if ( (self != 0) && (port != 0) )
   {

      apx_dataElement_t *dataElement;

      if (self->isFinalized == false)
      {
         int32_t errorLine;
         apx_node_finalize(self, &errorLine);
      }
      dataElement = port->dataSignature.dataElement;
      if ( (dataElement == 0) || (dataElement->baseType == APX_BASE_TYPE_NONE) || (dataElement->packLen == 0) )
      {
         apx_setError(APX_VALUE_ERROR);
         return -1;
      }
      adt_bytearray_resize(output, dataElement->packLen);
      if (port->portAttributes != 0)
      {
         apx_portAttributes_t *attr = port->portAttributes;
         if (attr->initValue == 0)
         {
            //if no init value is given, set to 0
            uint8_t *buf = adt_bytearray_data(output);
            memset(buf, 0, dataElement->packLen);
            return 0;
         }
         else
         {
            uint8_t *pBegin;
            uint8_t *pEnd;
            uint8_t *pResult;
            pBegin = adt_bytearray_data(output);
            pEnd = pBegin + dataElement->packLen;
            pResult = apx_dataElement_pack_dv(dataElement, pBegin, pEnd, attr->initValue);
            if ( (pResult == 0) || (pResult == pBegin) )
            {
               return -1;
            }
            return 0;
         }
      }
      else
      {
         //if no init value is given, set to 0
         uint8_t *buf = adt_bytearray_data(output);
         memset(buf, 0, dataElement->packLen);
         return 0;
      }
   }
   errno = EINVAL;
   return -1;
}
