#include "error_codes.h"

#include "common/constants.h"
#include "common/defines.h"
#include "common/macros.h"

// Error Code String Representation

#if PROGRAM_FEATURE_NO_ERROR_CODE_NAMES == 0
str ERROR_CODE_NAMES[] = {
    #define ERROR_CODE_ENTRY(name, value) "ERROR_CODE_" #name, // TODO: don't prefix error codes with "ERROR_CODE_" in the string representation, to save space
    #include "error_codes_inline.h"
    #undef ERROR_CODE_ENTRY
};
#else
str ERROR_CODE_NAMES[] = { "ERROR_CODE_UNDEFINED" };
#endif

// error handling interfaces

static struct {
    error_code error_code_globals_error_code;
    error_code error_code_globals_error_code_chain[32];
    u8 error_code_globals_error_code_chain_current;
} error_handler = {
    .error_code_globals_error_code = ERROR_CODE_EXECUTION_SUCCESSFUL,

    .error_code_globals_error_code_chain = { ERROR_CODE_EXECUTION_SUCCESSFUL },
    .error_code_globals_error_code_chain_current = 0
};

// Error Code Functions

error_code error_codes_get_error_code(error_code error) {
    return error & ERROR_CODE_MASK;
}

bool error_codes_is_flag_set(error_code error, error_flag flag) {
    return ((error & ERROR_FLAGS_MASK) >> ERROR_FLAGS_SHIFT) == flag;
}

bool error_codes_is_flag_or_lower(error_code error, error_flag flag) {
    return ((error & ERROR_FLAGS_MASK) >> ERROR_FLAGS_SHIFT) <= flag;
}

str error_codes_get_error_code_name(error_code error) {
    #if PROGRAM_FEATURE_NO_ERROR_CODE_NAMES == 0
    return ERROR_CODE_NAMES[error_codes_get_error_code(error)];
    #else
    return ERROR_CODE_NAMES[0];
    #endif
}

inline void error_codes_raise_error(error_code error) {
    error_handler.error_code_globals_error_code = error;
}

void error_codes_clear_error(void) {
    error_handler.error_code_globals_error_code = ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code error_codes_get_error(void) {
    return error_handler.error_code_globals_error_code;
}

void error_code_raise_chain_error(error_code error) {
    if (error_handler.error_code_globals_error_code_chain_current < ARRAY_LENGTH(error_handler.error_code_globals_error_code_chain)) {
        error_handler.error_code_globals_error_code_chain[error_handler.error_code_globals_error_code_chain_current] = error;
        error_handler.error_code_globals_error_code_chain_current++;
    }
}

void error_codes_clear_error_chain(void) {
    for (u8 i = 0; i < (u8) ARRAY_LENGTH(error_handler.error_code_globals_error_code_chain); i++) {
        error_handler.error_code_globals_error_code_chain[i] = ERROR_CODE_EXECUTION_SUCCESSFUL;
    }

    error_handler.error_code_globals_error_code_chain_current = 0;
}
