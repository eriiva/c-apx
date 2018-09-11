#ifndef APX_PORT_H
#define APX_PORT_H
//simple port - data port with one data element
#include "apx_dataSignature.h"
#include "apx_portAttributes.h"

#define APX_REQUIRE_PORT 0
#define APX_PROVIDE_PORT 1

typedef struct apx_port_tag{
	char *name;
	apx_dataSignature_t dataSignature; //Container for both raw and derived data signature
	apx_portAttributes_t *portAttributes; //port attributes object, includes the raw attributes string
	char *portSignature; //full port signature, excluding the initial 'R' or 'P'
	uint8_t portType; //APX_REQUIRE_PORT or APX_PROVIDE_PORT
	int32_t portIndex; //index of the port 0..len(ports) where it resides on its parent node
	int32_t lineNumber; //line number in the APX-file where this port is defines. Special value 0 is used in case this port was created without an APX-file
}apx_port_t;

/***************** Public Function Declarations *******************/

apx_error_t apx_port_create(apx_port_t *self, uint8_t portDirection, const char *name, const char* dataSignature, const char *attributes, int32_t lineNumber);
void apx_port_destroy(apx_port_t *self);
apx_port_t* apx_providePort_new(const char *name, const char* dataSignature, const char *attributes);
apx_port_t* apx_requirePort_new(const char *name, const char* dataSignature, const char *attributes);
void apx_port_delete(apx_port_t *self);
void apx_port_vdelete(void *arg);

#if 0
void apx_port_setDerivedDataSignature(apx_port_t *self, const char *dataSignature);
const char *apx_port_derivePortSignature(apx_port_t *self);
#endif
const char *apx_port_getPortSignature(apx_port_t *self);

int32_t apx_port_getPackLen(apx_port_t *self);
void apx_port_setPortIndex(apx_port_t *self, int32_t portIndex);
int32_t  apx_port_getPortIndex(apx_port_t *self);

#endif //APX_PORT_H
