#ifndef APX_ERROR_H
#define APX_ERROR_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_NO_ERROR                  0
#define APX_INVALID_ARGUMENT_ERROR    1
#define APX_MEM_ERROR                 2
#define APX_PARSE_ERROR               3
#define APX_DATA_DATA_SIGNATURE_ERROR 4
#define APX_VALUE_ERROR               5
#define APX_LENGTH_ERROR              6
#define APX_ELEMENT_TYPE_ERROR        7
#define APX_DV_TYPE_ERROR             8
#define APX_UNSUPPORTED_ERROR         9
#define APX_NOT_IMPLEMENTED_ERROR     10
#define APX_UNMATCHED_BRACE_ERROR     11
#define APX_UNMATCHED_BRACKET_ERROR   12
#define APX_UNMATCHED_STRING_ERROR    13
#define APX_INVALID_TYPE_REF_ERROR    14
#define APX_EXPECTED_BRACKET_ERROR    15

typedef int32_t apx_error_t;

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
apx_error_t apx_getLastError(void);
void apx_setError(apx_error_t error);
void apx_clearError(void);

#endif //APX_ERROR_H
