#ifndef WAVE_LANGUAGE_COMMON
#define WAVE_LANGUAGE_COMMON

// Includes

#include "common/constants.h"
#include "common/debug.h"
#include "common/defines.h"
#include "common/error_codes.h"
#include "common/macros.h"

#include "common/data/string/hash.h"

// Defines

#if PROGRAM_FEATURE_WAVE_COMPILER_DEBUG_MODE != 0
#define WAVE_COMPILER_DEBUG(format, ...) DEBUG_INFO(format, __VA_ARGS__)
#else
#define WAVE_COMPILER_DEBUG(...) EMPTY_CODE_BLOCK
#endif

// Typedefs

typedef enum {
    WAVE_TYPE_U8,
    WAVE_TYPE_U16,
    WAVE_TYPE_U32,
    WAVE_TYPE_U64,

    WAVE_TYPE_I8,
    WAVE_TYPE_I16,
    WAVE_TYPE_I32,
    WAVE_TYPE_I64,

    WAVE_TYPE_F32,
    WAVE_TYPE_F64,

    WAVE_TYPE_STR,
    WAVE_TYPE_ARR,

    WAVE_TYPE_ENUM,
    WAVE_TYPE_STRUCT,

    WAVE_TYPE_FUNC,

    WAVE_TYPE_VOID,

    WAVE_TYPE_NONE
} WAVE_TYPES;
typedef byte wave_type; // similar to @number_type

umax wave_type_get_size(wave_type type);
cstr wave_type_get_string(wave_type type);

// parameters for native functions

typedef struct {
    wave_type type;
    bool has_default_value;
    union_number default_value;
} wave_parameter;

// common function data

typedef struct {
    string_hash name;
    wave_type return_type;

    wave_parameter* parameters;
    u16 parameter_count;

    bool error_function;
} wave_function;

// memory functions

typedef error_code (*wave_memory_allocation_function) (void** out_pointer, umax size);
typedef error_code (*wave_memory_allocation_zero_function) (void** out_pointer, umax size);
typedef error_code (*wave_memory_reallocation_function) (void** in_out_pointer, umax size);
typedef error_code (*wave_memory_deallocation_function) (void* pointer);

// native functions

typedef error_code (*wave_native_function_callback) (const byte* stack_start, const byte* stack_end, byte* stack, byte** out_stack);

typedef struct {
    wave_function function_data;
    wave_native_function_callback callback;
} wave_native_function; // extends @wave_function

// Functions

bool wave_compiler_builtin_char_is_namespace(char character);

#endif
