#ifndef WAVE_LANGUAGE_OPCODES
#define WAVE_LANGUAGE_OPCODES

// Includes

#include "common/constants.h"
#include "common/macros.h"

// Enum Generation

#define WAVE_OPCODE_PREFIX_STRING "OPCODE_"

typedef enum {
    #define OPCODE_ENTRY(name) CONCAT(OPCODE_, name),
    #include "wave_opcodes_inline.h"

    #undef OPCODE_ENTRY

    OPCODE_MAX
} WAVE_OPCODES;
COMPILE_ASSERT(OPCODE_MAX <= 256, too_many_opcodes_defined);

typedef byte wave_opcode; // WAVE_OPCODES

typedef enum {
    DEBUG_INSTRUCTION_TYPE_FUNCTION_START,
    DEBUG_INSTRUCTION_TYPE_FUNCTION_END,

    DEBUG_INSTRUCTION_TYPE_SCOPE_START,
    DEBUG_INSTRUCTION_TYPE_SCOPE_END,

    DEBUG_INSTRUCTION_TYPE_MAX
} DEBUG_INSTRUCTION_TYPES;
COMPILE_ASSERT(DEBUG_INSTRUCTION_TYPE_MAX <= 256, too_many_debug_instruction_types_defined);
typedef byte debug_instruction_type;

// Opcode Functions

str wave_opcode_get_name(wave_opcode opcode);
str wave_opcode_get_complete_name(wave_opcode opcode);

#endif
