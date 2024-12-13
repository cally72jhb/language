#ifndef STANDARD_LIBRARY_UINT_MATH_INLINE
#define STANDARD_LIBRARY_UINT_MATH_INLINE

#include "common/constants.h"
#include "common/macros.h"

#endif

// Mode

#ifndef UINT_MODE_DECLARE
#define UINT_MODE_DECLARE (1)
#endif

// Data Type

#ifndef UINT
#define UINT u32
#endif

#ifndef UINT_MAX
#define UINT_MAX (U32_MAX)
#endif

// Defines

#if UINT_MODE_DECLARE == 0
#define INLINE_FUNCTION_BODY(...) __VA_ARGS__
#else
#define INLINE_FUNCTION_BODY(...) ;
#endif

// Math Functions

// min, max, clamp

UINT CAT(UINT, _min)(UINT value1, UINT value2) INLINE_FUNCTION_BODY({
    return value1 < value2 ? value1 : value2;
})

UINT CAT(UINT, _max)(UINT value1, UINT value2) INLINE_FUNCTION_BODY({
    return value1 > value2 ? value1 : value2;
})

UINT CAT(UINT, _clamp)(UINT value, UINT min, UINT max) INLINE_FUNCTION_BODY({
    return value < min ? min : (value > max ? max : value);
})

// pow

UINT CAT(UINT, _pow)(UINT base, UINT exponent) INLINE_FUNCTION_BODY({
    UINT result = 1;
    UINT temp = base;

    while (true) {
        if (exponent % 2) {
            result *= temp;
        }

        exponent /= 2;
        if (exponent == 0) {
            break;
        }

        temp *= temp;
    }

    return result;
})

// un-define macros

#undef UINT
#undef UINT_MAX

#undef INLINE_FUNCTION_BODY
