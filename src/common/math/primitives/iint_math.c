#include "iint_math.h"

// Generic Math Functions

#define UINT_MODE_DECLARE (0) /* define the function body */

// i8

#define IINT i8
#define IINT_MAX (I8_MAX)
#include "iint_math_inline.h"

// i16

#define IINT i16
#define IINT_MAX (I16_MAX)
#include "iint_math_inline.h"

// i32

#define IINT i32
#define IINT_MAX (I32_MAX)
#include "iint_math_inline.h"

// i64

#define IINT i64
#define IINT_MAX (I64_MAX)
#include "iint_math_inline.h"

#undef UINT_MODE_DECLARE
