#ifndef APX_ERROR_H
#define APX_ERROR_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define APX_NO_ERROR                  0 //RMF code: 500
#define APX_INVALID_ARGUMENT_ERROR    1 //RMF code: 501
#define APX_MEM_ERROR                 2 //RMF code: 502
#define APX_PARSE_ERROR               3 //RMF code: 503
#define APX_DATA_SIGNATURE_ERROR      4 //RMF code: 504
#define APX_VALUE_ERROR               5 //RMF code: 505
#define APX_LENGTH_ERROR              6 //RMF code: 506
#define APX_ELEMENT_TYPE_ERROR        7 //RMF code: 507
#define APX_DV_TYPE_ERROR             8 //RMF code: 508
#define APX_UNSUPPORTED_ERROR         9 //RMF code: 509
#define APX_NOT_IMPLEMENTED_ERROR     10 //RMF code: 510
#define APX_UNMATCHED_BRACE_ERROR     11 //RMF code: 511
#define APX_UNMATCHED_BRACKET_ERROR   12 //RMF code: 512
#define APX_UNMATCHED_STRING_ERROR    13 //RMF code: 513
#define APX_INVALID_TYPE_REF_ERROR    14 //RMF code: 514
#define APX_EXPECTED_BRACKET_ERROR    15 //RMF code: 515
#define APX_INVALID_ATTRIBUTE_ERROR   16 //RMF code: 516
#define APX_TOO_MANY_NODES_ERROR      17 //RMF code: 517
#define APX_NODE_MISSING_ERROR        18 //RMF code: 518
#define APX_NODE_ALREADY_EXISTS_ERROR 19 //RMF code: 519
#define APX_MISSING_BUFFER_ERROR      20 //RMF code: 520
#define APX_MISSING_FILE_ERROR        21 //RMF code: 521
#define APX_NAME_ERROR                22 //RMF code: 522 //should this be changed to APX_MISSING_NAME_ERROR?
#define APX_NAME_TOO_LONG_ERROR       23 //RMF code: 523
#define APX_NOT_FOUND_ERROR           24 //RMF code: 524

#define RMF_APX_NO_ERROR                  500
#define RMF_APX_INVALID_ARGUMENT_ERROR    501
#define RMF_APX_MEM_ERROR                 502
#define RMF_APX_PARSE_ERROR               503
#define RMF_APX_DATA_SIGNATURE_ERROR      504
#define RMF_APX_VALUE_ERROR               505
#define RMF_APX_LENGTH_ERROR              506
#define RMF_APX_ELEMENT_TYPE_ERROR        507
#define RMF_APX_DV_TYPE_ERROR             508
#define RMF_APX_UNSUPPORTED_ERROR         509
#define RMF_APX_NOT_IMPLEMENTED_ERROR     510
#define RMF_APX_UNMATCHED_BRACE_ERROR     511
#define RMF_APX_UNMATCHED_BRACKET_ERROR   512
#define RMF_APX_UNMATCHED_STRING_ERROR    513
#define RMF_APX_INVALID_TYPE_REF_ERROR    514
#define RMF_APX_EXPECTED_BRACKET_ERROR    515
#define RMF_APX_INVALID_ATTRIBUTE_ERROR   516
#define RMF_APX_TOO_MANY_NODES_ERROR      517
#define RMF_APX_NODE_MISSING_ERROR        518
#define RMF_APX_NODE_ALREADY_EXISTS_ERROR 519
#define RMF_APX_MISSING_BUFFER_ERROR      520
#define RMF_APX_MISSING_FILE_ERROR        521
#define RMF_APX_NAME_ERROR                522
#define RMF_APX_NAME_TOO_LONG_ERROR       523
#define RMF_APX_NOT_FOUND_ERROR           524

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
