#ifndef WAVE_LANGUAGE_DEBUG
#define WAVE_LANGUAGE_DEBUG

// Includes

#include "platform.h"

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/api/wave_api_include_header.h"

// Debug Functions

WAVE_VM_NATIVE_FUNCTION(debug, print, sizeof(addr),
    str string = NULL; STACK_POP(string);
    u32 length = 0;

    if (string == NULL) {
        length = 0;
    } else {
        length = *((u32*) string); // retrieve length of string
        string += sizeof(u32); // jump to the start of the string data
    }

    platform_commandline_print(string, length);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

WAVE_VM_NATIVE_FUNCTION(debug, println, sizeof(addr),
    str string = NULL; STACK_POP(string);
    u32 length = 0;

    if (string == NULL) {
        length = 0;
    } else {
        length = *((u32*) string); // retrieve length of string
        string += sizeof(u32); // jump to the start of the string data
    }

    platform_commandline_print(string, length);
    platform_commandline_print("\n", 1);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

#include "language/api/wave_api_include_end.h"

#endif
