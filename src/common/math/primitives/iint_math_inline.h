#ifndef STANDARD_LIBRARY_IINT_MATH_INLINE
#define STANDARD_LIBRARY_IINT_MATH_INLINE

#include "common/constants.h"
#include "common/macros.h"

#endif

// Mode

#ifndef UINT_MODE_DECLARE
#define UINT_MODE_DECLARE (1)
#endif

// Data Type

#ifndef IINT
#define IINT i32
#endif

#ifndef IINT_MAX
#define IINT_MAX (I32_MAX)
#endif

// Defines

#if UINT_MODE_DECLARE == 0
#define INLINE_FUNCTION_BODY(...) __VA_ARGS__
#else
#define INLINE_FUNCTION_BODY(...) ;
#endif

// Math Functions

// abs, neg

IINT CAT(IINT, _abs)(IINT value) INLINE_FUNCTION_BODY({
    return value < 0 ? -value : value;
})

IINT CAT(IINT, _neg)(IINT value) INLINE_FUNCTION_BODY({
    return value > 0 ? -value : value;
})

// min, max, clamp

IINT CAT(IINT, _min)(IINT value1, IINT value2) INLINE_FUNCTION_BODY({
    return value1 < value2 ? value1 : value2;
})

IINT CAT(IINT, _max)(IINT value1, IINT value2) INLINE_FUNCTION_BODY({
    return value1 > value2 ? value1 : value2;
})

IINT CAT(IINT, _clamp)(IINT value, IINT min, IINT max) INLINE_FUNCTION_BODY({
    return value < min ? min : (value > max ? max : value);
})

// signum, pow

IINT CAT(IINT, _signum)(IINT value) INLINE_FUNCTION_BODY({
    return (value > 0) ? +1 : ((value < 0) ? -1 : 0);
})

IINT CAT(IINT, _pow)(IINT base, IINT exponent) INLINE_FUNCTION_BODY({
    if (exponent < 0) {
        switch (base) {
            case 0: { return IINT_MAX; }
            case 1: { return 1; }
            case -1: { return exponent % 2 ? -1 : 1; }

            default: {
                return 0;
            }
        }
    }

    IINT result = 1;
    IINT temp = base;
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

#undef IINT
#undef IINT_MAX

#undef INLINE_FUNCTION_BODY
