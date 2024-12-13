#include "string.h"

#include "platform.h"

#include "common/constants.h"

#include "common/math/primitives/f32_math.h"
#include "common/math/primitives/f64_math.h"

// String Functions

bool char_is_letter(char character) {
    return character >= 'a' && character <= 'z' || character >= 'A' && character <= 'Z';
}

bool char_is_digit(char character) {
    return character >= '0' && character <= '9';
}

bool char_is_whitespace(char character) {
    return character == ' ' || character == '\f' || character == '\n' || character == '\r' || character == '\t' || character == '\v';
}

#if PLATFORM_BIT_COUNT == 32 || PLATFORM_BIT_COUNT == 64

#define STRING_POINTER_MASK (sizeof(u32) - 1)

#if PLATFORM_BIT_COUNT == 32
    #define STRING_LENGTH_MASK01 (0x01010101)
    #define STRING_LENGTH_MASK80 (0x80808080)
#elif PLATFORM_BIT_COUNT == 64
    #define STRING_LENGTH_MASK01 (0x0101010101010101)
    #define STRING_LENGTH_MASK80 (0x8080808080808080)
#endif

u32 str_length(str string) {
    char* pointer_8 = NULL;
    u32* pointer_32 = (u32*) (((u64) string) & ~STRING_POINTER_MASK);

    if (((u32) (*pointer_32 - STRING_LENGTH_MASK01)) & ((u32) ((~(*pointer_32)) & STRING_LENGTH_MASK80))) {
        pointer_32++;
        for (pointer_8 = string; pointer_8 < (char*) pointer_32; pointer_8++) {
            if (*pointer_8 == '\0') {
                return pointer_8 - string;
            }
        }
    } else {
        pointer_32++;
    }

    while (true) {
        if ((*pointer_32 - STRING_LENGTH_MASK01) & ((~(*pointer_32)) & STRING_LENGTH_MASK80)) {
            pointer_8 = (char*) pointer_32;

            if (pointer_8[0] == '\0') { return (pointer_8 - string) + 0; }
            if (pointer_8[1] == '\0') { return (pointer_8 - string) + 1; }
            if (pointer_8[2] == '\0') { return (pointer_8 - string) + 2; }
            if (pointer_8[3] == '\0') { return (pointer_8 - string) + 3; }

            #if (PLATFORM_BIT_COUNT >= 64)

            if (pointer_8[4] == '\0') { return (pointer_8 - string) + 4; }
            if (pointer_8[5] == '\0') { return (pointer_8 - string) + 5; }
            if (pointer_8[6] == '\0') { return (pointer_8 - string) + 6; }
            if (pointer_8[7] == '\0') { return (pointer_8 - string) + 7; }

            #endif

        }

        pointer_32++;
    }
}

#else

u32 str_length(str string) {
    u32 length = 0;
    for (; *string != '\0'; ++string, length++);
    return length;
}

#endif

bool str_is_equals(str string1, str string2) {
    do {
        if (*string1 == '\0') {
            return *string2 == '\0';
        } else if (*string2 == '\0') {
            return *string1 == '\0';
        }
    } while (*(string1++) == *(string2++));

    return false;
}

bool str_is_equals_quick(cstr string1, cstr string2, u32 string1_length, u32 string2_length) {
    if (string1_length != string2_length) {
        return false;
    }

    u32 i = 0;
    for (i = 0; (i + sizeof(u64)) < string1_length; i += sizeof(u64)) {
        if (*((u64*) string1) != *((u64*) string2)) {
            return false;
        }

        string1 += sizeof(u64);
        string2 += sizeof(u64);
    }

    #define STR_EQU(type, offset) (*((type*) (string1 + (offset))) == *((type*) (string2 + (offset))))

    switch (string1_length - i) {
        case (sizeof(u64)):                            { return STR_EQU(u64, 0);}
        case (sizeof(u32) + sizeof(u16) + sizeof(u8)): { return STR_EQU(u32, 0) && STR_EQU(u16, sizeof(u32)) && STR_EQU(u8, sizeof(u32) + sizeof(u16)); }
        case (sizeof(u32) + sizeof(u16)):              { return STR_EQU(u32, 0) && STR_EQU(u16, sizeof(u32)); }
        case (sizeof(u32) + sizeof(u8)):               { return STR_EQU(u32, 0) && STR_EQU(u8,  sizeof(u32)); }
        case (sizeof(u32)):                            { return STR_EQU(u32, 0); }
        case (sizeof(u16) + sizeof(u8)):               { return STR_EQU(u16, 0) && STR_EQU(u8,  sizeof(u16)); }
        case (sizeof(u16)):                            { return STR_EQU(u16, 0); }
        case (sizeof(u8)):                             { return STR_EQU(u8,  0); }

        default: {
            goto str_is_equals_quick_end;
        }
    }

    #undef STR_EQU

    str_is_equals_quick_end: {}

    return false;
}

bool str_is_empty(cstr string) {
    return string[0] == '\0';
}

bool str_starts_with(str string1, str string2) {
    while (*string1 == *string2) {
        if (*string2 == '\0') {
            return true;
        }

        string1++;
        string2++;
    }

    return *string2 == '\0';
}

#define GET_CHAR() (**format)
#define NEXT_CHAR() do { if (**format == '\0') { return false; } (*format)++; } while (0)

#define FORMAT_PRINT(character)                     \
    do {                                            \
        if (*in_out_result_length > max_length) {   \
            return false;                           \
        } else {                                    \
            if (print_format) {                     \
                (*(*result)) = character;           \
                (*result)++;                        \
            }                                       \
                                                    \
            (*in_out_result_length)++;              \
        }                                           \
    } while (0)

#define FORMAT_PRINT_RAW(character)                 \
    do {                                            \
        if (*in_out_result_length > max_length) {   \
            return false;                           \
        } else {                                    \
            (*(*result)) = character;               \
            (*result)++;                            \
        }                                           \
    } while (0)

#define FORMAT_PRINT_PADDING(offset, condition)     \
    do {                                            \
        if (format_pad_with_zero) {                 \
            for (u32 i = offset; condition; i++) {  \
                FORMAT_PRINT('0');                  \
            }                                       \
        } else if (has_sequence) {                  \
            str sequence = sequence_start;          \
            for (u32 i = offset; condition; i++) {  \
                if (sequence >= sequence_end) {     \
                sequence = sequence_start;          \
                }                                   \
                                                    \
                if (*sequence == '\\') {            \
                sequence++;                         \
                }                                   \
                                                    \
                FORMAT_PRINT(*sequence);            \
                sequence++;                         \
            }                                       \
        } else {                                    \
            for (u32 i = offset; condition; i++) {  \
            FORMAT_PRINT(' ');                      \
            }                                       \
        }                                           \
    } while (0)

static inline bool builtin_str_format_sequence(
    bool print_format, str* format, u32 max_length, str* result, u32* in_out_result_length, str_format_data* format_data, u32* in_out_format_data_current,
    bool has_sequence, str sequence_start, str sequence_end, bool format_string_length_arg
) {
    if (!has_sequence) {
        return true;
    }

    u32 length = 0;
    if (format_string_length_arg) {
        length = format_data[*in_out_format_data_current].u32;
        (*in_out_format_data_current)++;

        if (GET_CHAR() != '.') {
            (*format)--;
        }
    } else if (char_is_digit(GET_CHAR())) {
        while (char_is_digit(GET_CHAR())) {
            length = length * 10 + ((u32) (GET_CHAR() - '0'));
            NEXT_CHAR();
        }

        if (GET_CHAR() == '.') {
            NEXT_CHAR();
        }
    } else {
        return true;
    }

    if (length > 0) {
        str temp = NULL;
        for (u32 i = 0; i < length; i++) {
            temp = sequence_start;
            while (temp < sequence_end) {
                if (*temp == '\\') {
                    temp++;
                }

                FORMAT_PRINT(*temp);
                temp++;
            }
        }
    }

    return true;
}

static inline bool builtin_str_format_str(
    bool print_format, str* format, u32 max_length, str* result, u32* in_out_result_length, str_format_data* format_data, u32* in_out_format_data_current,
    bool format_align_left, u32 format_field_width, bool format_pad_with_zero, bool has_sequence, str sequence_start, str sequence_end, bool format_no_max_width, bool format_string_length_arg, bool format_type_dot
) {
    u32 length = U32_MAX;

    if (format_string_length_arg) {
        length = format_data[*in_out_format_data_current].u32;
        (*in_out_format_data_current)++;
    }

    str value = format_data[*in_out_format_data_current].string;
    (*in_out_format_data_current)++;

    // handle special cases

    if (value == NULL) {
        if (format_no_max_width) {
            FORMAT_PRINT('n');
            FORMAT_PRINT('u');
            FORMAT_PRINT('l');
            FORMAT_PRINT('l');
        } else {
            if (format_field_width > 0) { FORMAT_PRINT('n'); format_field_width--; }
            if (format_field_width > 0) { FORMAT_PRINT('u'); format_field_width--; }
            if (format_field_width > 0) { FORMAT_PRINT('l'); format_field_width--; }
            if (format_field_width > 0) { FORMAT_PRINT('l'); format_field_width--; }
        }

        return true;
    }

    // get length or print to string

    if (length == 0) {
        if (format_field_width > 0 && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(0, i < format_field_width);
        }

        return true;
    }

    if (format_field_width != U32_MAX) {

        // print string with given width

        if (format_align_left) {

            // print string left aligned

            u32 printed_characters = 0;

            for (u32 i = 0; (format_no_max_width ? true : (i < format_field_width)) && length > 0 && *value != '\0'; i++) {
                if (format_type_dot) {
                    if (*value != '\a' && *value != '\b' && *value != '\x1B' && *value != '\f' && *value != '\n' && *value != '\r' && *value != '\t' && *value != '\v') {
                        FORMAT_PRINT(*value);
                    } else {
                        FORMAT_PRINT(' ');
                    }
                } else {
                    FORMAT_PRINT(*value);
                }

                value++;
                length--;
                printed_characters++;
            }

            if (printed_characters < format_field_width) {
                FORMAT_PRINT_PADDING(0, i < format_field_width - printed_characters);
            }
        } else {

            // print string right aligned

            if (length == U32_MAX) {
                length = str_length(value);
            }

            if (format_field_width > length) {
                FORMAT_PRINT_PADDING(0, i < format_field_width - length);
            }

            for (u32 i = 0; (format_no_max_width ? true : (i < format_field_width)) && length > 0; i++) {
                if (format_type_dot) {
                    if (*value != '\a' && *value != '\b' && *value != '\x1B' && *value != '\f' && *value != '\n' && *value != '\r' && *value != '\t' && *value != '\v') {
                        FORMAT_PRINT(*value);
                    } else {
                        FORMAT_PRINT(' ');
                    }
                } else {
                    FORMAT_PRINT(*value);
                }

                value++;
                length--;
            }
        }
    } else {

        // print string without any width limitation

        if (!format_type_dot) {
            while (*value != '\0' && length > 0) {
                FORMAT_PRINT(*value);

                value++;
                length--;
            }
        } else {
            while (*value != '\0') {
                if (*value != '\a' && *value != '\b' && *value != '\x1B' && *value != '\f' && *value != '\n' && *value != '\r' && *value != '\t' && *value != '\v') {
                    FORMAT_PRINT(*value);
                } else {
                    FORMAT_PRINT(' ');
                }

                value++;
            }
        }
    }

    return true;
}

static inline bool builtin_str_format_uint(
    u64 value,
    bool print_format, str* format, u32 max_length, str* result, u32* in_out_result_length,
    bool format_align_left, u32 format_field_width, bool format_pad_with_zero, bool has_sequence, str sequence_start, str sequence_end, bool format_no_max_width, bool format_sign
) {
    u64 power = 1;
    u64 temp = value;
    u32 digits = 0;
    while (temp >= 10) {
        power *= 10;
        temp /= 10;
        digits++;
    }

    digits++;

    u32 offset = format_sign ? 1 : 0;

    if (!format_align_left) {
        if (format_sign && (format_pad_with_zero || has_sequence)) {
            FORMAT_PRINT('+');
        }

        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(offset, i < format_field_width - digits);
        }

        if (format_sign && !format_pad_with_zero && !has_sequence) {
            FORMAT_PRINT('+');
        }

        for (u32 i = offset; (format_no_max_width ? true : (i < format_field_width)) && power > 0; i++) {
            u64 digit = value / power;
            FORMAT_PRINT('0' + (char) digit);
            value -= digit * power;
            power /= 10;
        }
    } else {
        if (format_sign) {
            FORMAT_PRINT('+');
        }

        for (u32 i = offset; (format_no_max_width ? true : (i < format_field_width)) && power > 0; i++) {
            u64 digit = value / power;
            FORMAT_PRINT('0' + (char) digit);
            value -= digit * power;
            power /= 10;
        }

        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(offset, i < format_field_width - digits);
        }
    }

    return true;
}

static inline bool builtin_str_format_int(
    i64 value,
    bool print_format, str* format, u32 max_length, str* result, u32* in_out_result_length,
    bool format_align_left, u32 format_field_width, bool format_pad_with_zero, bool has_sequence, str sequence_start, str sequence_end, bool format_no_max_width, bool format_sign
) {
    bool negative = false;
    bool minimum = false;

    // handle special cases

    if (value == I64_MIN) {
        minimum = true;
        value = I64_MIN + 1;
    }

    if (value < 0) {
        value = -value;
        negative = true;
    }

    // print result

    u64 integer = (u64) value;

    u64 power = 1;
    u64 temp = integer;
    u32 digits = 0;
    while (temp >= 10) {
        power *= 10;
        temp /= 10;
        digits++;
    }

    digits++;

    bool print_sign = format_sign || negative;
    char sign = negative ? '-' : '+';
    u32 offset = print_sign ? 1 : 0;

    if (!format_align_left) {
        if (print_sign && format_pad_with_zero) {
            FORMAT_PRINT(sign);
        }

        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(offset, i < format_field_width - digits);
        }

        if (print_sign && !format_pad_with_zero) {
            FORMAT_PRINT(sign);
        }

        for (u32 i = offset; (format_no_max_width ? true : (i < format_field_width)) && power > 0; i++) {
            u64 digit = integer / power;

            if (minimum && power == 1) {
                FORMAT_PRINT('0' + (-(I64_MIN % 10)));
            } else {
                FORMAT_PRINT('0' + (char) digit);
            }

            integer -= digit * power;
            power /= 10;
        }
    } else {
        if (print_sign) {
            FORMAT_PRINT(sign);
        }

        for (u32 i = offset; (format_no_max_width ? true : (i < format_field_width)) && power > 0; i++) {
            u64 digit = integer / power;

            if (minimum && power == 1) {
                FORMAT_PRINT('0' + (-(I64_MIN % 10)));
            } else {
                FORMAT_PRINT('0' + (char) digit);
            }

            integer -= digit * power;
            power /= 10;
        }

        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(offset, i < format_field_width - digits);
        }
    }

    return true;
}

static inline bool builtin_str_format_hex(
    u64 value,
    bool print_format, str* format, u32 max_length, str* result, u32* in_out_result_length,
    bool format_align_left, u32 format_field_width, bool format_pad_with_zero, bool has_sequence, str sequence_start, str sequence_end, bool format_no_max_width, bool format_sign
) {
    cstr hex_digits = format_sign ? "0123456789ABCDEF" : "0123456789abcdef";

    // print result

    u64 temp = value;
    u32 digits = 0;
    if (temp == 0) {
        digits = 1;
    } else {
        while (temp > 0) {
            temp >>= 4;
            digits++;
        }
    }

    if (!format_align_left) {
        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(0, i < format_field_width - digits);
        }

        str start = *result;
        str end = NULL;

        do {
            FORMAT_PRINT((char) (hex_digits[value % 16]));
        } while ((value /= 16) > 0);

        if (print_format) {
            end = *result - 1;
            char prev = '\0';
            while (start < end) {
                prev = *start;
                *start++ = *end;
                *end-- = prev;
            }
        }
    } else {
        str start = *result;
        str end = NULL;

        do {
            FORMAT_PRINT((char) (hex_digits[value % 16]));
        } while ((value /= 16) > 0);

        if (print_format) {
            end = *result - 1;
            char prev = '\0';
            while (start < end) {
                prev = *start;
                *start++ = *end;
                *end-- = prev;
            }
        }

        if (digits < format_field_width && format_field_width != U32_MAX) {
            FORMAT_PRINT_PADDING(0, i < format_field_width - digits);
        }
    }

    return true;
}

#undef GET_CHAR
#undef NEXT_CHAR

#undef FORMAT_PRINT
#undef FORMAT_PRINT_RAW
#undef FORMAT_PRINT_PADDING

void str_format(str format, str_format_data* format_data, str out_string, u32* in_out_length) {
    bool print_format = out_string != NULL;
    str result = print_format ? out_string : NULL;
    u32 max_length = print_format ? *in_out_length : U32_MAX;

    u32 result_length = 0;
    u32 format_data_current = 0;

    // formatting options

    u32 format_field_width = 0; // the minimum and maximum length of the formatting type, if necessary the result is padded in front
    bool format_align_left = false; // the minimum and maximum length of the formatting type, if necessary the result is padded in front

    bool format_pad_with_zero = false; // whether to use zeros ('0') or spaces (' ') for padding
    str sequence_start = NULL;
    str sequence_end = NULL;
    bool has_sequence = false; // defines a custom character sequence to be used for padding

    bool format_no_max_width = false; // does not cap the result at the width that is specified

    bool format_string_length_arg = false; // for string format type: after the string argument there follows an u32 containing the length of the string to be printed
    bool format_type_dot = false; // for binary format type: numbers are printed with one dot ('.') every 8 bits; for string format type: prints newline, tab, ... as spaces (' ')

    bool format_sign = false; // for number formats: prints the sign of the number, even if it is positive
    bool format_prefix = false; // for binary and hex formats: prefixes the format with its corresponding type ("0x" or "0b")

    // macros

    #define GET_CHAR() (*format)
    #define NEXT_CHAR() do { if (*format == '\0') { if (print_format) { *result = '\0'; } *in_out_length = result_length; return; } format++; } while (0)

    #define FORMAT_PRINT(character)             \
        do {                                    \
            if (result_length > max_length) {   \
                if (print_format) {             \
                    *result = '\0';             \
                }                               \
                                                \
                *in_out_length = result_length; \
                return;                         \
            } else if (print_format) {          \
                *(result++) = character;        \
            }                                   \
                                                \
            result_length++;                    \
        } while (0)

    // formatting loop

    while (GET_CHAR() != '\0' && (!print_format || result_length < max_length)) {
        if (GET_CHAR() != '%') {
            FORMAT_PRINT(GET_CHAR());
            NEXT_CHAR();
            continue;
        } else if (GET_CHAR() == '%') {
            NEXT_CHAR();

            // parse formatting options

            format_field_width = U32_MAX;
            format_align_left = false;

            format_pad_with_zero = false;
            sequence_start = NULL;
            sequence_end = NULL;
            has_sequence = false;

            format_no_max_width = false;

            format_string_length_arg = false;
            format_type_dot = false;

            format_sign = false;
            format_prefix = false;

            // parse formatting options

            if (GET_CHAR() == '0') {
                format_pad_with_zero = true;
                NEXT_CHAR();
            } else if (GET_CHAR() == '{') {
                NEXT_CHAR();

                has_sequence = true;
                sequence_start = format;

                while (GET_CHAR() != '}' && GET_CHAR() != '\0') {
                    if (GET_CHAR() == '\\') {
                        NEXT_CHAR();
                        if (GET_CHAR() == '}') {
                            NEXT_CHAR();
                            continue;
                        }
                    }

                    NEXT_CHAR();
                }

                if (GET_CHAR() == '\0') {
                    if (print_format) {
                        *result = '\0';
                    }

                    *in_out_length = result_length;
                    return;
                }

                sequence_end = format;
                NEXT_CHAR();

                if (sequence_end == sequence_start) {
                    has_sequence = false;
                }
            }

            if (GET_CHAR() == '<') {
                format_align_left = true;
                NEXT_CHAR();
            } else if (GET_CHAR() == '>') {
                format_align_left = false;
                NEXT_CHAR();
            }

            if (GET_CHAR() == '+') {
                format_sign = true;
                NEXT_CHAR();
            }

            if (GET_CHAR() == ':') {
                format_no_max_width = true;
                NEXT_CHAR();
            }

            switch (GET_CHAR()) {
                case ('.'): {
                    format_type_dot = true;
                    NEXT_CHAR();
                    break;
                }

                case ('*'): {
                    format_string_length_arg = true;
                    NEXT_CHAR();
                    break;
                }

                case ('#'): {
                    format_prefix = true;
                    NEXT_CHAR();
                    break;
                }

                case ('0'): case ('1'): case ('2'): case ('3'): case ('4'): case ('5'): case ('6'): case ('7'): case ('8'): case ('9'): {
                    format_field_width = 0;
                    while (char_is_digit(GET_CHAR())) {
                        format_field_width = format_field_width * 10 + ((u32) (GET_CHAR() - '0'));
                        NEXT_CHAR();
                    }

                    break;
                }

                default: {
                    break;
                }
            }

            // macros

            #define RUN_BUILTIN_FUNCTION(function, ...) \
                do {                                    \
                    if (!function(__VA_ARGS__)) {       \
                        if (print_format) {             \
                            *result = '\0';             \
                        }                               \
                                                        \
                        *in_out_length = result_length; \
                        return;                         \
                    }                                   \
                } while (0)

            // parse formats

            switch (GET_CHAR()) {
                case ('%'): {
                    FORMAT_PRINT('%');
                    break;
                }

                // do nothing

                case ('n'): {
                    format_data_current++;
                    break;
                }

                // repeat sequence

                case ('r'): {
                    NEXT_CHAR();
                    RUN_BUILTIN_FUNCTION(builtin_str_format_sequence, print_format, &format, max_length, &result, &result_length, format_data, &format_data_current, has_sequence, sequence_start, sequence_end, format_string_length_arg);
                    break;
                }

                case ('s'): {
                    RUN_BUILTIN_FUNCTION(
                        builtin_str_format_str,
                        print_format, &format, max_length, &result, &result_length, format_data, &format_data_current,
                        format_align_left, format_field_width, format_pad_with_zero, has_sequence, sequence_start, sequence_end, format_no_max_width, format_string_length_arg, format_type_dot
                    );

                    break;
                }

                case ('u'):
                case ('i'):
                case ('x'): {
                    char value_type = GET_CHAR();

                    u64 u64_value = 0;
                    i64 i64_value = 0;

                    bool skip_character = false;
                    if (*(format + 1) != '\0') {
                        NEXT_CHAR();
                        skip_character = true;
                    }

                    switch (GET_CHAR()) {
                        case ('8'): {
                            u64_value = (u64) format_data[format_data_current].u8;
                            i64_value = (i64) format_data[format_data_current].i8;
                            format_data_current++;
                            break;
                        }

                        case ('1'): {
                            NEXT_CHAR();
                            if (GET_CHAR() == '6') {
                                u64_value = (u64) format_data[format_data_current].u16;
                                i64_value = (i64) format_data[format_data_current].i16;
                                format_data_current++;
                                break;
                            } else {
                                format--;
                                goto str_format_type_number_default;
                            }
                        }

                        case ('3'): {
                            NEXT_CHAR();
                            if (GET_CHAR() == '2') {
                                u64_value = (u64) format_data[format_data_current].u32;
                                i64_value = (i64) format_data[format_data_current].i32;
                                format_data_current++;
                                break;
                            } else {
                                format--;
                                goto str_format_type_number_default;
                            }
                        }

                        case ('6'): {
                            NEXT_CHAR();
                            if (GET_CHAR() == '4') {
                                u64_value = (u64) format_data[format_data_current].u64;
                                i64_value = (i64) format_data[format_data_current].i64;
                                format_data_current++;
                                break;
                            } else {
                                format--;
                                goto str_format_type_number_default;
                            }
                        }

                        str_format_type_number_default:
                        default: {
                            u64_value = (u64) format_data[format_data_current].u32;
                            i64_value = (i64) format_data[format_data_current].i32;
                            format_data_current++;

                            if (skip_character) {
                                format--;
                            }

                            break;
                        }
                    }

                    if (value_type == 'u') {
                        RUN_BUILTIN_FUNCTION(
                            builtin_str_format_uint,
                            u64_value,
                            print_format, &format, max_length, &result, &result_length,
                            format_align_left, format_field_width, format_pad_with_zero, has_sequence, sequence_start, sequence_end, format_no_max_width, format_sign
                        );
                    } else if (value_type == 'i') {
                        RUN_BUILTIN_FUNCTION(
                            builtin_str_format_int,
                            i64_value,
                            print_format, &format, max_length, &result, &result_length,
                            format_align_left, format_field_width, format_pad_with_zero, has_sequence, sequence_start, sequence_end, format_no_max_width, format_sign
                        );
                    } else if (value_type == 'x') {
                        RUN_BUILTIN_FUNCTION(
                            builtin_str_format_hex,
                            u64_value,
                            print_format, &format, max_length, &result, &result_length,
                            format_align_left, format_field_width, format_pad_with_zero, has_sequence, sequence_start, sequence_end, format_no_max_width, format_sign
                        );
                    }

                    break;
                }

                case ('c'): {
                    char value = format_data[format_data_current].character;
                    format_data_current++;

                    FORMAT_PRINT(value);
                    break;
                }

                // TODO

                case ('v'): {
                    bool value = format_data[format_data_current].boolean;
                    format_data_current++;

                    // print or determine length

                    if (value) {
                        FORMAT_PRINT('t');
                        FORMAT_PRINT('r');
                        FORMAT_PRINT('u');
                        FORMAT_PRINT('e');
                    } else {
                        FORMAT_PRINT('f');
                        FORMAT_PRINT('a');
                        FORMAT_PRINT('l');
                        FORMAT_PRINT('s');
                        FORMAT_PRINT('e');
                    }

                    break;
                }

                default: {
                    break;
                }
            }

            #undef RUN_BUILTIN_FUNCTION

            NEXT_CHAR();
            continue;
        }
    }

    #undef GET_CHAR
    #undef NEXT_CHAR

    #undef FORMAT_PRINT

    *in_out_length = result_length;
}

void str_format_old(str format, str_format_data* variables, str out_string, u32* in_out_length) {
    bool print = out_string != NULL;
    str result = print ? out_string : NULL;
    u32 max_length = print ? *in_out_length : U32_MAX;

    u32 result_len = 0;
    u32 element = 0;

    // state variables

    bool negative = false;
    bool minimum = false;

    str start = NULL;
    str end = NULL;

    // formatting options

    u32 format_field_width = 0; // the minimum and maximum length of the formatting type, if necessary the result is padded in front
    bool format_align_left = false; // the minimum and maximum length of the formatting type, if necessary the result is padded in front

    bool format_pad_with_zero = false; // whether to use zeros ('0') or spaces (' ') for padding

    bool format_binary_separator = false; // for binary format type: numbers are printed with one dot ('.') every 8 bits
    bool format_string_length_arg = false; // for string format type: after the string argument there follows an u32 containing the length of the string to be printed

    bool format_prefix = false; // for binary and hex formats: prefixes the format with its corresponding type ("0x" or "0b")

    // Printing Macro (safe & "unsafe" version)

    #define CHECK_FORMAT() do { if (*format == '\0') { if (print) { *result = '\0'; } *in_out_length = result_len; return; } } while (0)

    #define FORMAT_PRINT(character) do { if (result_len > max_length) { if (print) { *result = '\0'; } *in_out_length = result_len; return; } else if (print) { *(result++) = character; } result_len++; } while (0)
    #define FORMAT_PRINT_RAW(character) do { if (result_len < max_length) { *(result++) = character; result_len++; } else { *result = '\0'; *in_out_length = result_len; return; } } while (0)

    //#define FORMAT_PRINT(character) do { if (print) { *(result++) = character; } result_len++; } while (0)
    //#define FORMAT_PRINT_RAW(character) do { *(result++) = character; result_len++; } while (0)

    // Format Loop

    while (*format != '\0' && (!print || result_len < max_length)) {
        if (*format != '%') {
            FORMAT_PRINT(*format);

            format++;
            continue;
        } else if (*format == '%') {
            format++;
            CHECK_FORMAT();

            // parse formatting options

            format_field_width = U32_MAX;

            format_pad_with_zero = false;

            format_binary_separator = false;
            format_string_length_arg = false;

            format_prefix = false;

            // parse formatting options

            if (*format == '0') {
                format_pad_with_zero = true;

                format++;
                CHECK_FORMAT();
            }

            if (*format == '-') {
                format_align_left = true;

                format++;
                CHECK_FORMAT();
            }

            switch (*format) {
                case ('.'): {
                    format_binary_separator = true;

                    format++;
                    CHECK_FORMAT();
                    break;
                }

                case ('*'): {
                    format_string_length_arg = true;

                    format++;
                    CHECK_FORMAT();
                    break;
                }

                case ('#'): {
                    format_prefix = true;

                    format++;
                    CHECK_FORMAT();
                    break;
                }

                case ('0'): case ('1'): case ('2'): case ('3'): case ('4'): case ('5'): case ('6'): case ('7'): case ('8'): case ('9'): {
                    format_field_width = 0;
                    while (char_is_digit(*format)) {
                        format_field_width = format_field_width * 10 + ((u32) (*format - '0'));
                        format++;
                        CHECK_FORMAT();
                    }

                    break;
                }

                default: {
                    break;
                }
            }

            // individual formats

            switch (*format) {
                case ('%'): {
                    FORMAT_PRINT('%');
                    break;
                }

                case ('n'): {
                    element++;
                    break;
                }

                case ('{'): {
                    format++;
                    CHECK_FORMAT();

                    str sequence_start = format;
                    str sequence_end = NULL;

                    while (*format != '}') {
                        format++;
                        CHECK_FORMAT();
                    }

                    sequence_end = format - 1;
                    format++;
                    CHECK_FORMAT();

                    u32 length = 0;
                    if (char_is_digit(*format)) {
                        while (char_is_digit(*format)) {
                            length = length * 10 + ((u32) (*format - '0'));
                            format++;
                            CHECK_FORMAT();
                        }
                    } else if (*format == '.') {
                        length = variables[element].u32;
                        element++;
                    } else {
                        format--;
                    }

                    if (length > 0) {
                        str temp = NULL;
                        for (u32 i = 0; i < length; i++) {
                            temp = sequence_start;
                            while (temp <= sequence_end) {
                                FORMAT_PRINT(*temp);
                                temp++;
                            }
                        }
                    }

                    break;
                }

                case ('u'): {
                    u64 value = 0;

                    // parse size

                    format++;
                    CHECK_FORMAT();
                    if (*format == '3') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '2') {
                            format--;
                            goto str_format_type_uint_end;
                        } else {
                            value = (u64) variables[element].u32;
                        }
                    } else if (*format == '6') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '4') {
                            format--;
                            goto str_format_type_uint_end;
                        } else {
                            value = variables[element].u64;
                        }
                    } else {
                        value = (u64) variables[element].u32;
                        format--;
                    }

                    // next element

                    element++;

                    // reset state variables

                    start = result;
                    end = NULL;

                    // print result

                    u64 power = 1;
                    u64 temp = value;
                    u32 digits = 0;
                    while (temp >= 10) {
                        power *= 10;
                        temp /= 10;
                        digits++;
                    }

                    digits++;

                    if (!format_align_left) {
                        if (digits < format_field_width && format_field_width != U32_MAX) {
                            if (format_pad_with_zero) {
                                for (u32 i = 0; i < format_field_width - digits; i++) {
                                    FORMAT_PRINT('0');
                                }
                            } else {
                                for (u32 i = 0; i < format_field_width - digits; i++) {
                                    FORMAT_PRINT(' ');
                                }
                            }
                        }

                        for (u32 i = 0; i < format_field_width && power > 0; i++) {
                            u64 digit = value / power;
                            FORMAT_PRINT('0' + (char) digit);
                            value -= digit * power;
                            power /= 10;
                        }
                    } else {
                        for (u32 i = 0; i < format_field_width && power > 0; i++) {
                            u64 digit = value / power;
                            FORMAT_PRINT('0' + (char) digit);
                            value -= digit * power;
                            power /= 10;
                        }

                        if (digits < format_field_width && format_field_width != U32_MAX) {
                            if (format_pad_with_zero) {
                                for (u32 i = 0; i < format_field_width - digits; i++) {
                                    FORMAT_PRINT('0');
                                }
                            } else {
                                for (u32 i = 0; i < format_field_width - digits; i++) {
                                    FORMAT_PRINT(' ');
                                }
                            }
                        }
                    }

                    str_format_type_uint_end: {}
                    break;
                }

                case ('i'): {
                    i64 value = 0;

                    // parse size

                    format++;
                    CHECK_FORMAT();
                    if (*format == '3') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '2') {
                            format--;
                            goto str_format_type_int_end;
                        } else {
                            value = (i64) variables[element].i32;
                        }
                    } else if (*format == '6') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '4') {
                            format--;
                            goto str_format_type_int_end;
                        } else {
                            value = variables[element].i64;
                        }
                    } else {
                        value = (i64) variables[element].i32;
                        format--;
                    }

                    element++;

                    // reset state variables

                    negative = false;
                    minimum = false;

                    // handle special cases

                    if (value == I64_MIN) {
                        minimum = true;
                        value = I64_MIN + 1;
                    }

                    if (value < 0) {
                        value = -value;
                        negative = true;
                    }

                    // print prefix

                    if (negative) {
                        FORMAT_PRINT('-');
                    }

                    // reset state variables

                    start = result;
                    end = NULL;

                    // special case

                    if (minimum) {
                        FORMAT_PRINT('0' + (-(I64_MIN % 10)));

                        value /= 10;
                    }

                    // get length or print to string

                    do {
                        FORMAT_PRINT((char) ('0' + value % 10));
                    } while ((value /= 10) > 0);

                    // reverse string

                    if (print) {
                        end = result - 1;

                        char prev = '\0';
                        while (start < end) {
                            prev = *start;
                            *start++ = *end;
                            *end-- = prev;
                        }
                    }

                    str_format_type_int_end: {}

                    break;
                }

                case ('f'): {
                    f32 value_f32 = 0.0F;
                    f64 value_f64 = 0.0;

                    bool float_64_bit = false;

                    // parse size

                    format++;
                    CHECK_FORMAT();
                    if (*format == '3') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '2') {
                            format--;
                            goto str_format_type_float_end;
                        } else {
                            value_f32 = variables[element].f32;
                            float_64_bit = false;
                        }
                    } else if (*format == '6') {
                        format++;
                        CHECK_FORMAT();
                        if (*format != '4') {
                            format--;
                            goto str_format_type_float_end;
                        } else {
                            value_f64 = variables[element].f64;
                            float_64_bit = true;
                        }
                    }

                    // common

                    u64 fraction = 0;

                    // parse requested precision

                    u32 precision = 0;
                    bool print_fraction = true;
                    bool print_zero = true;

                    if (*(format + 1) == '0') {
                        print_fraction = false;
                        format++;
                        CHECK_FORMAT();
                    } else if (char_is_digit(*(format + 1))) {
                        format++;
                        CHECK_FORMAT();

                        while (char_is_digit(*format)) {
                            precision = precision * 10 + (char) (*format - '0');
                            format++;
                            CHECK_FORMAT();
                        }
                        precision = precision > STR_FORMAT_F32_MAX_PRECISION ? STR_FORMAT_F32_MAX_PRECISION : precision;

                        format--;
                    } else {
                        print_zero = false;
                        precision = STR_FORMAT_F32_MAX_PRECISION;
                    }

                    if (!float_64_bit) {
                        goto str_format_type_f32_start;
                    } else {
                        goto str_format_type_f64_start;
                    }

                    // Type F32

                    str_format_type_f32_start: {}

                    element++;

                    // handle special cases

                    if (value_f32 != value_f32 || value_f32 == F32_NAN) {
                        if (print) {
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('A');
                            FORMAT_PRINT_RAW('N');
                        } else {
                            result_len += 3;
                        }

                        break;
                    } else if (value_f32 > F32_MAX || value_f32 == F32_POS_INF) {
                        if (print) {
                            FORMAT_PRINT_RAW('+');
                            FORMAT_PRINT_RAW('I');
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('F');
                        } else {
                            result_len += 4;
                        }

                        break;
                    } else if (value_f32 < F32_MIN || value_f32 == F32_NEG_INF) {
                        if (print) {
                            FORMAT_PRINT_RAW('-');
                            FORMAT_PRINT_RAW('I');
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('F');
                        } else {
                            result_len += 4;
                        }

                        break;
                    }

                    negative = false;

                    if (value_f32 < 0.0 && value_f32 != -0.0) {
                        value_f32 = -value_f32;
                        negative = true;
                    } else {
                        negative = false;
                    }

                    // set temporary variables

                    f32 f32_integer = 0.0F;
                    f32 f32_fraction = 0.0F;
                    f32_part(value_f32, &f32_integer, &f32_fraction);

                    f32 f32_pow_precision = f32_pow10((f32) precision);

                    // store fractional part in integer

                    f32 f32_temp_num = (value_f32 - f32_integer) * f32_pow_precision;
                    fraction = (u64) f32_temp_num;
                    if ((f32_temp_num - (f32) fraction) >= 0.5F) {
                        fraction++;

                        if ((f32) fraction >= f32_pow_precision) {
                            //integer_part++;
                            fraction = 0;
                        }
                    }

                    goto str_format_type_float_print_common;

                    // Type F64

                    str_format_type_f64_start: {}

                    element++;

                    // handle special cases

                    if (value_f64 != value_f64 || value_f64 == F64_NAN) {
                        if (print) {
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('A');
                            FORMAT_PRINT_RAW('N');
                        } else {
                            result_len += 3;
                        }

                        break;
                    } else if (value_f64 > F64_MAX || value_f64 == F64_POS_INF) {
                        if (print) {
                            FORMAT_PRINT_RAW('+');
                            FORMAT_PRINT_RAW('I');
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('F');
                        } else {
                            result_len += 4;
                        }

                        break;
                    } else if (value_f64 < F64_MIN || value_f64 == F64_NEG_INF) {
                        if (print) {
                            FORMAT_PRINT_RAW('-');
                            FORMAT_PRINT_RAW('I');
                            FORMAT_PRINT_RAW('N');
                            FORMAT_PRINT_RAW('F');
                        } else {
                            result_len += 4;
                        }

                        break;
                    }

                    negative = false;

                    if (value_f64 < 0.0 && value_f64 != -0.0) {
                        value_f64 = -value_f64;
                        negative = true;
                    } else {
                        negative = false;
                    }

                    // set temporary variables

                    f64 f64_integer = 0.0;
                    f64 f64_fraction = 0.0;
                    f64_part(value_f64, &f64_integer, &f64_fraction);

                    f64 f64_pow_precision = f64_pow10((f64) precision);

                    // store fractional part in integer

                    f64 f64_temp_num = (value_f64 - f64_integer) * f64_pow_precision;
                    fraction = (u64) f64_temp_num;
                    if ((f64_temp_num - (f64) fraction) >= 0.5) {
                        fraction++;

                        if ((f64) fraction >= f64_pow_precision) {
                            //integer_part++;
                            fraction = 0;
                        }
                    }

                    goto str_format_type_float_print_common;

                    // common

                    str_format_type_float_print_common: {}

                    if (!print_zero) {
                        while (fraction % 10 == 0 && fraction != 0) {
                            fraction /= 10;
                        }
                    }

                    // print sign

                    if (negative) {
                        FORMAT_PRINT('-');
                    }

                    // print integer part

                    if (!float_64_bit) {
                        f32 divisor = 1.0F;
                        while (f32_integer / divisor >= 10.0) {
                            divisor *= 10.0F;
                        }

                        while (divisor >= 1.0F) {
                            char digit = (char) (f32_integer / divisor);
                            FORMAT_PRINT('0' + digit);
                            f32_integer -= ((f32) digit) * divisor;
                            divisor /= 10.0F;
                        }
                    } else {
                        f64 divisor = 1.0;
                        while (f64_integer / divisor >= 10.0) {
                            divisor *= 10.0;
                        }

                        while (divisor >= 1.0) {
                            char digit = (char) (f64_integer / divisor); // TODO: this prints big numbers as x99999999x making them hard to read round the result and add a setting to not round
                            FORMAT_PRINT('0' + digit);
                            f64_integer -= ((f64) digit) * divisor;
                            divisor /= 10.0;
                        }
                    }

                    // set temporary variables

                    start = result;
                    end = NULL;

                    // print fraction & integer part

                    if (print_fraction) {
                        u32 i = 0;
                        do {
                            FORMAT_PRINT((char) ('0' + fraction % 10));
                            i++;
                        } while ((fraction /= 10) > 0);

                        if (print_zero) {
                            while (i < precision) {
                                FORMAT_PRINT('0');
                                i++;
                            }
                        }

                        FORMAT_PRINT('.');
                    }

                    // reverse string

                    if (print) {
                        end = result - 1;

                        char prev = '\0';
                        while (start < end) {
                            prev = *start;
                            *start++ = *end;
                            *end-- = prev;
                        }
                    }

                    str_format_type_float_end: {}

                    break;
                }

                case ('s'): {
                    u32 length = U32_MAX;

                    if (format_string_length_arg) {
                        length = variables[element].u32;
                        element++;

                        if (length == 0) {
                            break;
                        }
                    }

                    str value = variables[element].string;
                    element++;

                    // handle special cases

                    if (value == NULL) {
                        if (print) {
                            FORMAT_PRINT_RAW('n');
                            FORMAT_PRINT_RAW('u');
                            FORMAT_PRINT_RAW('l');
                            FORMAT_PRINT_RAW('l');
                        } else {
                            result_len += 4;
                        }

                        break;
                    }

                    // get length or print to string

                    if (format_field_width != U32_MAX) {

                        // print string with given width

                        if (!format_align_left) {
                            u32 printed_characters = 0;
                            for (u32 i = 0; i < format_field_width && length > 0 && *value != '\0'; i++) {
                                FORMAT_PRINT(*value);

                                value++;
                                length--;
                                printed_characters++;
                            }

                            for (u32 i = 0; i < format_field_width - printed_characters; i++) {
                                FORMAT_PRINT(format_pad_with_zero ? '0' : ' ');
                            }
                        } else {
                            if (length == U32_MAX) {
                                length = str_length(value);
                            }

                            if (format_field_width > length) {
                                for (u32 i = 0; i < format_field_width - length; i++) {
                                    FORMAT_PRINT(format_pad_with_zero ? '0' : ' ');
                                }
                            }

                            for (u32 i = 0; i < format_field_width && length > 0; i++) {
                                FORMAT_PRINT(*value);

                                value++;
                                length--;
                            }
                        }
                    } else {
                        while (*value != '\0' && length > 0) {
                            FORMAT_PRINT(*value);

                            value++;
                            length--;
                        }
                    }

                    if (!format_binary_separator) {

                        // print string with spaces



                    } else {
                        while (*value != '\0') {
                            if (*value != '\a' && *value != '\b' && *value != '\x1B' && *value != '\f' && *value != '\n' && *value != '\r' && *value != '\t' && *value != '\v') {
                                FORMAT_PRINT(*value);
                            } else {
                                FORMAT_PRINT(' ');
                            }

                            value++;
                        }
                    }

                    break;
                }

                case ('c'): {
                    char value = variables[element].character;
                    element++;

                    FORMAT_PRINT(value);

                    break;
                }

                case ('x'): {
                    if (format_prefix) {
                        FORMAT_PRINT('0');
                        FORMAT_PRINT('x');
                    }

                    #define FORMAT_PRINT_HEX(type, type_name)                               \
                        do {                                                                \
                            type value = variables[element].type;                           \
                            element++;                                                      \
                                                                                            \
                            start = result;                                                 \
                            end = NULL;                                                     \
                                                                                            \
                            do {                                                            \
                                FORMAT_PRINT((char) (("0123456789ABCDEF")[value % 16]));    \
                            } while ((value /= 16) > 0);                                    \
                                                                                            \
                            if (print) {                                                    \
                                end = result - 1;                                           \
                                char prev = '\0';                                           \
                                while (start < end) {                                       \
                                    prev = *start;                                          \
                                    *start++ = *end;                                        \
                                    *end-- = prev;                                          \
                                }                                                           \
                            }                                                               \
                        } while (0)

                    format++;
                    CHECK_FORMAT();

                    switch (*format) {
                        case ('8'): {
                            FORMAT_PRINT_HEX(u8, U8);
                            break;
                        }

                        case ('1'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '6') {
                                FORMAT_PRINT_HEX(u16, U16);
                                break;
                            } else {
                                format--;
                                goto str_format_type_hex_default;
                            }
                        }

                        case ('3'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '2') {
                                FORMAT_PRINT_HEX(u32, U32);
                                break;
                            } else {
                                format--;
                                goto str_format_type_hex_default;
                            }
                        }

                        case ('6'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '4') {
                                FORMAT_PRINT_HEX(u64, U64);
                                break;
                            } else {
                                format--;
                                goto str_format_type_hex_default;
                            }
                        }

                        str_format_type_hex_default:
                        default: {
                            FORMAT_PRINT_HEX(u8, U8);
                            break;
                        }
                    }

                    #undef FORMAT_PRINT_HEX
                    break;
                }

                case ('p'): {
                    umax value = variables[element].umax;
                    element++;

                    // reset state variables

                    start = result;
                    end = NULL;

                    // get length or print to string

                    do {
                        FORMAT_PRINT((char) (("0123456789ABCDEF")[value % 16]));
                    } while ((value /= 16) > 0);

                    // reverse string

                    if (print) {
                        end = result - 1;

                        char prev = '\0';
                        while (start < end) {
                            prev = *start;
                            *start++ = *end;
                            *end-- = prev;
                        }
                    }

                    break;
                }

                case ('b'): {
                    if (format_prefix) {
                        FORMAT_PRINT('0');
                        FORMAT_PRINT('b');
                    }

                    #define FORMAT_PRINT_BIN(type, type_name)                                                                           \
                        do {                                                                                                            \
                            type value = variables[element].type;                                                                       \
                            element++;                                                                                                  \
                                                                                                                                        \
                            for (u32 i = 0; i < type_name##_BIT_COUNT; i++, value <<= 1) {                                              \
                                char character = (char) ('0' + ((value & type_name##_LEFTMOST_BIT) >> (type_name##_BIT_COUNT - 1)));    \
                                                                                                                                        \
                                if (format_binary_separator && i % 8 == 0 && i != 0) {                                                  \
                                    FORMAT_PRINT('.');                                                                                  \
                                }                                                                                                       \
                                                                                                                                        \
                                FORMAT_PRINT(character);                                                                                \
                            }                                                                                                           \
                        } while (0)

                    format++;
                    CHECK_FORMAT();

                    switch (*format) {
                        case ('8'): {
                            FORMAT_PRINT_BIN(u8, U8);
                            break;
                        }

                        case ('1'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '6') {
                                FORMAT_PRINT_BIN(u16, U16);
                                break;
                            } else {
                                goto str_format_type_bin_default;
                            }
                        }

                        case ('3'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '2') {
                                FORMAT_PRINT_BIN(u32, U32);
                                break;
                            } else {
                                goto str_format_type_bin_default;
                            }
                        }

                        case ('6'): {
                            format++;
                            CHECK_FORMAT();
                            if (*format == '4') {
                                FORMAT_PRINT_BIN(u64, U64);
                                break;
                            } else {
                                goto str_format_type_bin_default;
                            }
                        }

                        str_format_type_bin_default:
                        default: {
                            FORMAT_PRINT_BIN(u8, U8);
                            break;
                        }
                    }

                    #undef FORMAT_BIN_PRINT

                    break;
                }

                case ('v'): {
                    bool value = variables[element].boolean;
                    element++;

                    // print or determine length

                    if (value) {
                        if (print) {
                            FORMAT_PRINT_RAW('t');
                            FORMAT_PRINT_RAW('r');
                            FORMAT_PRINT_RAW('u');
                            FORMAT_PRINT_RAW('e');
                        } else {
                            result_len += 4;
                        }
                    } else {
                        if (print) {
                            FORMAT_PRINT_RAW('f');
                            FORMAT_PRINT_RAW('a');
                            FORMAT_PRINT_RAW('l');
                            FORMAT_PRINT_RAW('s');
                            FORMAT_PRINT_RAW('e');
                        } else {
                            result_len += 5;
                        }
                    }

                    break;
                }

                default: {
                    break;
                }
            }

            format++;
            CHECK_FORMAT();
            continue;
        }
    }

    #undef CHECK_FORMAT

    #undef FORMAT_PRINT
    #undef FORMAT_PRINT_RAW

    *in_out_length = result_len;
}
