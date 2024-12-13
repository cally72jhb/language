#include "platform.h"

#include "debug.h"

#include "constants.h"
#include "error_codes.h"

// debug interface

struct {
    byte* buffer;
    umax buffer_size;
} debug_interface;

// Functions

error_code debug_create() {
    debug_interface.buffer = NULL;
    debug_interface.buffer_size = 256;
    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_allocate_clear, (void**) &debug_interface.buffer, sizeof(byte) * debug_interface.buffer_size);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code debug_destroy() {
    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_deallocate, (void*) debug_interface.buffer);
    debug_interface.buffer = NULL;
    debug_interface.buffer_size = 0;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code debug_builtin_get_buffer(byte** out_buffer, umax size) {
    if (size > debug_interface.buffer_size) {
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_reallocate, (void**) &debug_interface.buffer, sizeof(byte) * size);
        debug_interface.buffer_size = size;
    }

    *out_buffer = debug_interface.buffer;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
