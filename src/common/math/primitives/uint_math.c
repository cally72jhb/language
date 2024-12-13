#include "uint_math.h"

// Generic Math Functions

#define UINT_MODE_DECLARE (0) /* define the function body */

// u8

#define UINT u8
#define UINT_MAX (U8_MAX)
#include "uint_math_inline.h"

// u16

#define UINT u16
#define UINT_MAX (U16_MAX)
#include "uint_math_inline.h"

// u32

#define UINT u32
#define UINT_MAX (U32_MAX)
#include "uint_math_inline.h"

// u64

#define UINT u64
#define UINT_MAX (U64_MAX)
#include "uint_math_inline.h"

#undef UINT_MODE_DECLARE
