#ifndef APX_CFG_H
#define APX_CFG_H

#ifndef APX_BUF_GROW_SIZE
# define APX_BUF_GROW_SIZE 65536
#endif

#ifndef APX_MAX_NAME_LEN
# define APX_MAX_NAME_LEN 256
#endif

#ifndef APX_MAX_PSG_LEN
# define APX_MAX_PSG_LEN 1024
#endif

#ifndef APX_MAX_DEFINITION_SIZE
# define APX_MAX_DEFINITION_SIZE 0x4000000 //64MB
#endif

#ifndef APX_DEFAULT_THREAD_STACK_SIZE
# define APX_DEFAULT_THREAD_STACK_SIZE 0x100000 //1MB
#endif

#ifndef APX_PORT_ID_TYPE
# define APX_PORT_ID_TYPE int32_t //valid selections are: int8_t, int16_t and int32_t. int32_t is default
#endif

#ifndef APX_MAX_NUM_MESSAGES
# define APX_MAX_NUM_MESSAGES 1000
#endif


#ifndef APX_MAX_NUM_EVENTS
# define APX_MAX_NUM_EVENTS 1000
#endif

#endif //APX_CFG_H
