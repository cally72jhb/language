#ifndef WAVE_LANGUAGE_MATH
#define WAVE_LANGUAGE_MATH

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "common/math/primitives/f32_math.h"
#include "common/math/primitives/f64_math.h"

#include "language/api/wave_api_include_header.h"

// Math Functions

WAVE_VM_NATIVE_FUNCTION(math, f32_sin, sizeof(f32),
    f32 value = 0.0F; STACK_GET_F32(value, 0);
    STACK_SET_TYPE(f32, f32_sin(value));
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

WAVE_VM_NATIVE_FUNCTION(math, f32_cos, sizeof(f32),
    f32 value = 0.0F; STACK_GET_F32(value, 0);
    STACK_SET_TYPE(f32, f32_cos(value));
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

WAVE_VM_NATIVE_FUNCTION(math, f32_tan, sizeof(f32),
    f32 value = 0.0F; STACK_GET_F32(value, 0);
    STACK_SET_TYPE(f32, f32_tan(value));
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

WAVE_VM_NATIVE_FUNCTION(math, f32_atan2, sizeof(f32) * 2,
    f32 value1 = 0.0F; STACK_GET_F32(value1, sizeof(f32));
    f32 value2 = 0.0F; STACK_GET_F32(value2, 0);
    STACK_PUSH_2BACK(f32, f32_atan2(value1, value2));
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
)

#include "language/api/wave_api_include_end.h"

#endif
