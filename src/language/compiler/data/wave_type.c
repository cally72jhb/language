#include "wave_type.h"

#include "wave_compiler_common.h"

// Type Functions

wave_type token_get_wave_type(wave_token token) {
    switch (token) {
        case WAVE_TOKEN_KEYWORD_BOOL:
        case WAVE_TOKEN_KEYWORD_U8:  { return WAVE_TYPE_U8; }
        case WAVE_TOKEN_KEYWORD_U16: { return WAVE_TYPE_U16; }
        case WAVE_TOKEN_KEYWORD_UINT:
        case WAVE_TOKEN_KEYWORD_U32: { return WAVE_TYPE_U32; }
        case WAVE_TOKEN_KEYWORD_U64: { return WAVE_TYPE_U64; }

        case WAVE_TOKEN_KEYWORD_CHAR:
        case WAVE_TOKEN_KEYWORD_I8:  { return WAVE_TYPE_I8; }
        case WAVE_TOKEN_KEYWORD_I16: { return WAVE_TYPE_I16; }
        case WAVE_TOKEN_KEYWORD_INT:
        case WAVE_TOKEN_KEYWORD_I32: { return WAVE_TYPE_I32; }
        case WAVE_TOKEN_KEYWORD_I64: { return WAVE_TYPE_I64; }

        case WAVE_TOKEN_KEYWORD_F32: { return WAVE_TYPE_F32; }
        case WAVE_TOKEN_KEYWORD_F64: { return WAVE_TYPE_F64; }

        #if PROGRAM_FEATURE_32BIT_MODE != 0
        case WAVE_TOKEN_KEYWORD_UMAX: { return WAVE_TYPE_U32; }
        #else
        case WAVE_TOKEN_KEYWORD_UMAX: { return WAVE_TYPE_U64; }
        #endif

        case WAVE_TOKEN_KEYWORD_STR: { return WAVE_TYPE_STR; }

        case WAVE_TOKEN_KEYWORD_ENUM: { return WAVE_TYPE_ENUM; }
        case WAVE_TOKEN_KEYWORD_STRUCT: { return WAVE_TYPE_STRUCT; }

        case WAVE_TOKEN_KEYWORD_FUNC: { return WAVE_TYPE_FUNC; }

        case WAVE_TOKEN_KEYWORD_VOID: { return WAVE_TYPE_VOID; }

        default: {
            return WAVE_TYPE_NONE;
        }
    }
}

wave_type wave_type_get_higher(wave_type type1, wave_type type2) {
    return type1 > type2 ? type1 : type2;
}
