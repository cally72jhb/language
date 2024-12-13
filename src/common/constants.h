#ifndef STANDARD_LIBRARY_CONSTANTS
#define STANDARD_LIBRARY_CONSTANTS

#include "defines.h"

// Primitive Data Types

// Integers

#if defined(__int8) && defined(__int16) && defined(__int32) && defined(__int64)
typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
#else
typedef signed char          i8;
typedef signed short         i16;
typedef signed int           i32;
typedef signed long long int i64;

typedef unsigned char          u8;
typedef unsigned short         u16;
typedef unsigned int           u32;
typedef unsigned long long int u64;
#endif

// Floats

typedef float  f32;
typedef double f64;

// Biggest Possible Integer

#ifdef size_t
typedef size_t umax;
#elif PROGRAM_FEATURE_32BIT_MODE == 0
typedef u64 umax;
#else
typedef u32 umax;
#endif

// Other

typedef u8 byte;
typedef u16 wchar;

typedef char* str;
typedef const char* cstr;

typedef void* addr;

// Conversion

typedef enum {
    NUMBER_TYPE_U8,
    NUMBER_TYPE_U16,
    NUMBER_TYPE_U32,
    NUMBER_TYPE_U64,

    NUMBER_TYPE_I8,
    NUMBER_TYPE_I16,
    NUMBER_TYPE_I32,
    NUMBER_TYPE_I64,

    NUMBER_TYPE_F32,
    NUMBER_TYPE_F64,

    NUMBER_TYPE_NONE
} PRIMITIVE_TYPES;
typedef byte number_type; // PRIMITIVE_TYPES

typedef union {
    u8 value_u8;
    u16 value_u16;
    u32 value_u32;
    u64 value_u64;

    i8 value_i8;
    i16 value_i16;
    i32 value_i32;
    i64 value_i64;

    f32 value_f32;
    f64 value_f64;

    umax value_umax;
} union_number;

typedef struct {
    number_type number_type;
    union_number number_value;
} number;

typedef union { u8 value_u8;   i8 value_i8;   } u8_i8;
typedef union { u16 value_u16; i16 value_i16; } u16_i16;
typedef union { u32 value_u32; i32 value_i32; } u32_i32;
typedef union { u64 value_u64; i64 value_i64; } u64_i64;

typedef union { u32 value_u32; f32 value_f32; } u32_f32;
typedef union { u64 value_u64; f64 value_f64; } u64_f64;

// Defines

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

// Floating Constants

#define HUGE_NUM (1E+300)

#define F32_INF ((f32) (HUGE_NUM * HUGE_NUM))
#define F32_NAN (F32_INF * 0.0F)
#define F64_INF ((f64) (HUGE_NUM * HUGE_NUM))
#define F64_NAN (F64_INF * 0.0)

#define F32_POS_INF (+F32_INF)
#define F32_POS_NAN (+F32_NAN)
#define F32_NEG_INF (-F32_INF)
#define F32_NEG_NAN (-F32_NAN)

#define F64_POS_INF (+F64_INF)
#define F64_POS_NAN (+F64_NAN)
#define F64_NEG_INF (-F64_INF)
#define F64_NEG_NAN (-F64_NAN)

// Integer Constants

#define I8_MIN (-128)
#define I8_MAX (127)
#define U8_MIN (0)
#define U8_MAX (0xFF)

#define I16_MIN (-32768)
#define I16_MAX (32767)
#define U16_MIN (0)
#define U16_MAX (0xFFFF)

#define I32_MIN (-2147483647 - 1)
#define I32_MAX (2147483647)
#define U32_MIN (0)
#define U32_MAX (0xFFFFFFFF)

#define I64_MIN (-9223372036854775807 - 1)
#define I64_MAX (9223372036854775807)
#define U64_MIN (0)
#define U64_MAX (0xFFFFFFFFFFFFFFFF)

#define F32_MIN (-3.402823466E+38F)
#define F32_MAX (+3.402823466E+38F)

#define F64_MIN (-1.7976931348623157E+308)
#define F64_MAX (+1.7976931348623157E+308)

// Integer Bit Count & Size

#define U8_BIT_COUNT (sizeof(u8) * 8)
#define U8_LEFTMOST_BIT (((u8) 0b1) << (U8_BIT_COUNT - 1))

#define U16_BIT_COUNT (sizeof(u16) * 8)
#define U16_LEFTMOST_BIT (((u16) 0b1) << (U16_BIT_COUNT - 1))

#define U32_BIT_COUNT (sizeof(u32) * 8)
#define U32_LEFTMOST_BIT (((u32) 0b1) << (U32_BIT_COUNT - 1))

#define U64_BIT_COUNT (sizeof(u64) * 8)
#define U64_LEFTMOST_BIT (((u64) 0b1) << (U64_BIT_COUNT - 1))

// Integer Bits

#define U8_BIT_0 ((u8) 0b0)
#define U8_BIT_1 (~U8_BIT_0)

#define U16_BIT_0 ((u16) 0b0)
#define U16_BIT_1 (~U16_BIT_0)

#define U32_BIT_0 ((u32) 0b0)
#define U32_BIT_1 (~U32_BIT_0)

#define U64_BIT_0 ((u64) 0b0)
#define U64_BIT_1 (~U64_BIT_0)

// umax

#define UMAX_MIN (0)
#if PROGRAM_FEATURE_32BIT_MODE == 0
#define UMAX_MAX (U64_MAX)
#define UMAX_BIT_COUNT (U64_BIT_COUNT)
#define UMAX_LEFTMOST_BIT (U64_LEFTMOST_BIT)
#define UMAX_BIT_0 (U64_BIT_0)
#define UMAX_BIT_1 (U64_BIT_1)
#else
#define UMAX_MAX (U32_MAX)
#define UMAX_BIT_COUNT (U32_BIT_COUNT)
#define UMAX_LEFTMOST_BIT (U32_LEFTMOST_BIT)
#define UMAX_BIT_0 (U32_BIT_0)
#define UMAX_BIT_1 (U32_BIT_1)
#endif

#endif
