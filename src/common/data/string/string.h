#ifndef STANDARD_LIBRARY_STRING
#define STANDARD_LIBRARY_STRING

// Includes

#include "common/constants.h"

// Typedefs

typedef char* str;
typedef const char* cstr;

typedef union {
    umax temp; // do not use; this is just for aligning the union properly

    u8 u8;
    u16 u16;
    u32 u32;
    u64 u64;

    i8 i8;
    i16 i16;
    i32 i32;
    i64 i64;

    f32 f32;
    f64 f64;

    umax umax;

    byte byte;
    char character;
    bool boolean;

    str string;
    cstr const_string;
    void* pointer;
    const void* const_pointer;
} str_format_data;

// Defines

#define STR_FORMAT_F32_MAX_PRECISION (8)
#define STR_FORMAT_F64_MAX_PRECISION (13)

// String Functions

bool char_is_letter(char character);
bool char_is_digit(char character);
bool char_is_whitespace(char character);

u32 str_length(str string);

bool str_is_empty(cstr string);
bool str_is_equals(str string1, str string2);
bool str_is_equals_quick(cstr string1, cstr string2, u32 string1_length, u32 string2_length);
bool str_starts_with(str string1, str string2);

void str_format(str format, str_format_data* variables, str out_string, u32* in_out_length);

#endif
