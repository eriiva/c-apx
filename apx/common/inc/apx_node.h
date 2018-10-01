#ifndef APX_NODE_H
#define APX_NODE_H
#include <stdint.h>
#include "adt_ary.h"
#include "adt_bytearray.h"
#include "apx_dataType.h"
#include "apx_port.h"
#include "apx_attributeParser.h"
#include "apx_error.h"
#include <stdbool.h>

struct apx_nodeInfo_tag;

typedef struct apx_node_tag{
   adt_ary_t datatypeList;
   adt_ary_t requirePortList;
   adt_ary_t providePortList;
   char *name;
   struct apx_nodeInfo_tag *nodeInfo;
   bool isFinalized;
   apx_attributeParser_t attributeParser;
} apx_node_t;


struct apx_portref_tag;
/***************** Public Function Declarations *******************/
//constructor/destructor
apx_node_t *apx_node_new(const char *name);
void apx_node_delete(apx_node_t *self);
void apx_node_vdelete(void *arg);
void apx_node_create(apx_node_t *self,const char *name);
void apx_node_destroy(apx_node_t *self);

//node functions
void apx_node_setName(apx_node_t *self, const char *name);
const char *apx_node_getName(apx_node_t *self);

//datatype functions
apx_datatype_t *apx_node_createDataType(apx_node_t *self, const char* name, const char *dsg, const char *attr, int32_t lineNumber);
//port functions
apx_port_t *apx_node_createRequirePort(apx_node_t *self, const char* name, const char *dsg, const char *attr, int32_t lineNumber);
apx_port_t *apx_node_createProvidePort(apx_node_t *self, const char* name, const char *dsg, const char *attr, int32_t lineNumber);
apx_error_t apx_node_finalize(apx_node_t *self, int32_t *errorLine);
apx_port_t *apx_node_getRequirePort(apx_node_t *self, int32_t portIndex);
apx_port_t *apx_node_getProvidePort(apx_node_t *self, int32_t portIndex);
int32_t apx_node_getNumRequirePorts(apx_node_t *self);
int32_t apx_node_getNumProvidePorts(apx_node_t *self);
int32_t apx_node_calcOutPortDataLen(apx_node_t *self);
int32_t apx_node_calcInPortDataLen(apx_node_t *self);
adt_bytearray_t *apx_node_createPortInitData(apx_node_t *self, apx_port_t *port);
apx_error_t apx_node_fillPortInitData(apx_node_t *self, apx_port_t *port, adt_bytearray_t *output);


#endif //APX_NODE_H
