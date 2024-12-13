#ifndef WAVE_LANGUAGE_TYPE
#define WAVE_LANGUAGE_TYPE

// Includes

#include "wave_compiler_common.h"

// Type Functions

wave_type token_get_wave_type(wave_token token);

wave_type wave_type_get_higher(wave_type type1, wave_type type2);

#endif
