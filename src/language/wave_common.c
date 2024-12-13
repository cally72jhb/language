#include "wave_common.h"

#include "common/constants.h"
#include "common/data/string/string.h"

// Functions

umax wave_type_get_size(wave_type type) {
    switch (type) {
        case WAVE_TYPE_U8:
        case WAVE_TYPE_I8: {
            return sizeof(u8);
        }

        case WAVE_TYPE_U16:
        case WAVE_TYPE_I16: {
            return sizeof(u16);
        }

        case WAVE_TYPE_U32:
        case WAVE_TYPE_I32:
        case WAVE_TYPE_F32: {
            return sizeof(u32);
        }

        case WAVE_TYPE_U64:
        case WAVE_TYPE_I64:
        case WAVE_TYPE_F64: {
            return sizeof(u64);
        }

        case WAVE_TYPE_FUNC: {
            return sizeof(u32);
        }

        case WAVE_TYPE_STR:
        case WAVE_TYPE_ARR:

        case WAVE_TYPE_ENUM:
        case WAVE_TYPE_STRUCT: {
            return sizeof(addr);
        }

        case WAVE_TYPE_VOID:
        default: {
            return 0;
        }
    }
}

cstr wave_type_get_string(wave_type type) {
    switch (type) {
        case WAVE_TYPE_U8:  { return "u8"; }
        case WAVE_TYPE_U16: { return "u16"; }
        case WAVE_TYPE_U32: { return "u32"; }
        case WAVE_TYPE_U64: { return "u64"; }

        case WAVE_TYPE_I8:  { return "i8"; }
        case WAVE_TYPE_I16: { return "i16"; }
        case WAVE_TYPE_I32: { return "i32"; }
        case WAVE_TYPE_I64: { return "i64"; }

        case WAVE_TYPE_F32: { return "f32"; }
        case WAVE_TYPE_F64: { return "f64"; }

        case WAVE_TYPE_STR: { return "str"; }
        case WAVE_TYPE_ARR: { return "arr"; }

        case WAVE_TYPE_ENUM: { return "enum"; }
        case WAVE_TYPE_STRUCT: { return "struct"; }

        case WAVE_TYPE_FUNC: { return "func"; }

        default: {
            return "none";
        }
    }
}

// other

bool wave_compiler_builtin_char_is_namespace(char character) {
    return char_is_letter(character) || char_is_digit(character) || character == '_';
}
