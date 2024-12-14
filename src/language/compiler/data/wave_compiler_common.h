#ifndef WAVE_LANGUAGE_COMPILER_COMMON
#define WAVE_LANGUAGE_COMPILER_COMMON

// Includes

#include "common/constants.h"

#include "language/wave_common.h"

// Typedefs

typedef enum {
    USAGE_TYPE_NONE,

    USAGE_TYPE_BINARY = 0b1 << 0,
    USAGE_TYPE_UNARY  = 0b1 << 1,

    USAGE_TYPE_PREFIX = 0b1 << 2,
    USAGE_TYPE_POSTFIX = 0b1 << 3,

    USAGE_TYPE_ASSIGN = 0b1 << 4,

    USAGE_TYPE_INDEX = 0b1 << 5, // array or string
} WAVE_TOKEN_OPERATOR_USAGE_TYPE;

typedef enum {
    WAVE_TOKEN_FILE_END,

    // keywords, operators and values

    #define KEYWORD_TOKENS (1)
    #define VALUE_TOKENS (1)
    #define OPERATOR_TOKENS (1)
    #define TOKEN_ENTRY(token_name, string_data, ...) CONCAT(WAVE_TOKEN_, token_name),
    #include "wave_token_list_inline.h"

    // other

    WAVE_TOKEN_INVALID,
    WAVE_TOKEN_UNDEFINED,

    WAVE_TOKEN_MAX
} WAVE_TOKENS;
COMPILE_ASSERT(WAVE_TOKEN_MAX <= 256, too_many_tokens_defined);
typedef byte wave_token; // WAVE_TOKENS

typedef struct {
    wave_token token; // the token
    u32 data_index; // the tokens index to the data stack // TODO: add binary tree or tree-like structure for storing identifiers

    u32 line; // the line in which the token has started
    u32 row; // the row in @line that points to the first character of the token
} parse_token;

typedef struct {
    f32 f32; // 32bit precision
    f64 f64; // 64bit precision
} wave_float; // for quick access and errors when type converting from one to the other

typedef struct {
    string_hash hash; // identifier string hash
    str source_pointer; // pointer to the source code
} wave_identifier;

// Defines

#define TOKEN_STACK_GROW_SIZE (1024)
#define DATA_STACK_GROW_SIZE (1024)

#define BYTECODE_STACK_GROW_SIZE (1024)

#define LOCALS_STACK_GROW_SIZE (16)
#define GLOBALS_STACK_GROW_SIZE (16)

#define FUNCTION_STACK_GROW_SIZE (16)
#define EXTERN_FUNCTION_STACK_GROW_SIZE (16)

#endif
