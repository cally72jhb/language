#include "wave_opcodes.h"

#include "common/constants.h"
#include "common/macros.h"

// Opcode String Representation

#if PROGRAM_FEATURE_NO_OPCODE_NAMES == 0
str WAVE_OPCODE_NAMES[] = {
    #define OPCODE_ENTRY(name) WAVE_OPCODE_PREFIX_STRING #name,
    #include "wave_opcodes_inline.h"

    #undef OPCODE_ENTRY
};
#else
str WAVE_OPCODE_NAMES[] = { "OPCODE_UNDEFINED" };
#endif

// Opcode Functions

str wave_opcode_get_name(wave_opcode opcode) {
    #if PROGRAM_FEATURE_NO_OPCODE_NAMES == 0
    return WAVE_OPCODE_NAMES[opcode] + (STRING_LENGTH(WAVE_OPCODE_PREFIX_STRING) - 1);
    #else
    return WAVE_OPCODE_NAMES[0];
    #endif
}

str wave_opcode_get_complete_name(wave_opcode opcode) {
    #if PROGRAM_FEATURE_NO_OPCODE_NAMES == 0
    return WAVE_OPCODE_NAMES[opcode];
    #else
    return WAVE_OPCODE_NAMES[0];
    #endif
}
