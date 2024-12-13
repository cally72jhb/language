#ifndef STANDARD_LIBRARY_ERROR_CODES
#define STANDARD_LIBRARY_ERROR_CODES

// Includes

#include "common/constants.h"
#include "common/defines.h"
#include "common/macros.h"

// Defines

#define ERROR_FLAGS_SHIFT (14)
#define ERROR_FLAGS_MASK (U16_MAX << ERROR_FLAGS_SHIFT)
#define ERROR_CODE_MASK (~ERROR_FLAGS_MASK)

typedef enum {
    ERROR_FLAG_IGNORE = 0,
    ERROR_FLAG_WARNING,
    ERROR_FLAG_SEVERE,
    ERROR_FLAG_FATAL,

    ERROR_FLAG_MAX
} error_flag;
COMPILE_ASSERT(ERROR_FLAG_MAX <= 4, too_many_error_flags_defined);

typedef enum {
    SHIFTED_ERROR_FLAG_IGNORE  = ERROR_FLAG_IGNORE  << ERROR_FLAGS_SHIFT,
    SHIFTED_ERROR_FLAG_WARNING = ERROR_FLAG_WARNING << ERROR_FLAGS_SHIFT,
    SHIFTED_ERROR_FLAG_SEVERE  = ERROR_FLAG_SEVERE  << ERROR_FLAGS_SHIFT,
    SHIFTED_ERROR_FLAG_FATAL   = ERROR_FLAG_FATAL   << ERROR_FLAGS_SHIFT
} error_flag_shifted;

// Generate Enum with Flags and String Representation

enum TEMP_ERROR_CODES {
    #define ERROR_CODE_ENTRY(name, value) CONCAT(TEMP_ERROR_CODE_, name),
    #include "error_codes_inline.h"
    #undef ERROR_CODE_ENTRY
};

typedef enum {
    #define ERROR_CODE_ENTRY(name, value) CONCAT(ERROR_CODE_, name) = CONCAT(TEMP_ERROR_CODE_, name) | (value << ERROR_FLAGS_SHIFT),
    #include "error_codes_inline.h"
    #undef ERROR_CODE_ENTRY

    ERROR_CODES_MAX
} ERROR_CODES;
typedef u16 error_code; // ERROR_CODES

// Error Code Functions

error_code error_codes_get_error_code(error_code error);
bool error_codes_is_flag_set(error_code error, error_flag flag);
bool error_codes_is_flag_or_lower(error_code error, error_flag flag);
str error_codes_get_error_code_name(error_code error);

void error_codes_raise_error(error_code error);
void error_codes_clear_error(void);
error_code error_codes_get_error();

void error_code_raise_chain_error(error_code error);
void error_code_clear_error_chain(void);

// Defines

#define RUN_ERROR_CODE_FUNCTION_IGNORE(function_name, ...)  \
    do {                                                    \
        function_name(EVAL(__VA_ARGS__));                   \
    } while (0)

#define RUN_ERROR_CODE_FUNCTION_TRACELESS(function_name, ...)                           \
    do {                                                                                \
        error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));  \
        if (function_result_##function_name != ERROR_CODE_EXECUTION_SUCCESSFUL) {       \
            return function_result_##function_name;                                     \
        }                                                                               \
    } while (0)

#define RUN_ERROR_CODE_FUNCTION_IGNORE_FLAG(function_name, flag, ...)                   \
    do {                                                                                \
        error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));  \
        if (!error_codes_is_flag_or_lower(function_result_##function_name, flag)) {     \
            return function_result_##function_name;                                     \
        }                                                                               \
    } while (0)

#define RUN_ERROR_CODE_FUNCTION_RAISE(function_name, ...)                               \
    do {                                                                                \
        error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));  \
        if (function_result_##function_name != ERROR_CODE_EXECUTION_SUCCESSFUL) {       \
            error_codes_raise_error(function_result_##function_name);                   \
        }                                                                               \
    } while (0)

#define RUN_ERROR_CODE_FUNCTION_RAISE_IGNORE_FLAG(function_name, flag, ...)             \
    do {                                                                                \
        error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));  \
        if (!error_codes_is_flag_or_lower(function_result_##function_name, flag)) {     \
            error_codes_raise_error(function_result_##function_name);                   \
        }                                                                               \
    } while (0)

#define RUN_ERROR_CODE_FUNCTION_CHAIN(function_name, ...)                               \
    do {                                                                                \
        error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));  \
        if (function_result_##function_name != ERROR_CODE_EXECUTION_SUCCESSFUL) {       \
            error_codes_raise_to_chain(function_result_##function_name);                \
        }                                                                               \
    } while (0)

#if PROGRAM_FEATURE_DEBUG_MODE == 0

#define RUN_ERROR_CODE_FUNCTION(function_name, ...) RUN_ERROR_CODE_FUNCTION_TRACELESS(function_name, __VA_ARGS__)

#else

#include "common/debug.h"

// Stacktrace Implementation

#if PROGRAM_FEATURE_STACK_TRACE != 0
    #if PROGRAM_FEATURE_STACK_TRACE_FUNCTIONS != 0
        #define RUN_ERROR_CODE_FUNCTION(function_name, ...)                                         \
            do {                                                                                    \
                DEBUG_FUNCTION_EXECUTION(function_name, __VA_ARGS__);                               \
                error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));      \
                if (function_result_##function_name != ERROR_CODE_EXECUTION_SUCCESSFUL) {           \
                    DEBUG_STACKTRACE(function_name, function_result_##function_name, __VA_ARGS__);  \
                    return function_result_##function_name;                                         \
                }                                                                                   \
            } while (0)
    #else
        #define RUN_ERROR_CODE_FUNCTION(function_name, ...)                                         \
            do {                                                                                    \
                error_code function_result_##function_name = function_name(EVAL(__VA_ARGS__));      \
                if (function_result_##function_name != ERROR_CODE_EXECUTION_SUCCESSFUL) {           \
                    DEBUG_STACKTRACE(function_name, function_result_##function_name, __VA_ARGS__);  \
                    return function_result_##function_name;                                         \
                }                                                                                   \
            } while (0)
    #endif
#else
    #define RUN_ERROR_CODE_FUNCTION(function_name, ...) RUN_ERROR_CODE_FUNCTION_TRACELESS(function_name, __VA_ARGS__)
#endif

#endif

#endif // STANDARD_LIBRARY_ERROR_CODES
