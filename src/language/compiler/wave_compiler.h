#ifndef WAVE_LANGUAGE_WAVE_COMPILER
#define WAVE_LANGUAGE_WAVE_COMPILER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"
#include "common/debug.h"
#include "common/defines.h"

#include "language/runtime/wave_vm.h"

// Defines

#if PROGRAM_FEATURE_NO_CONSOLE_COLORS == 0
#define COMPILER_ERROR_PREFIX ANSI_COLOR_NRM_RED "error" ANSI_COLOR_NRM_BLK " in " ANSI_COLOR_NRM_CYN "%s(%u:%u)" ANSI_COLOR_NRM_BLK " :: " ANSI_COLOR_NRM_YEL "func %s()" ANSI_COLOR_NRM_BLK ": " ANSI_COLOR_RESET
#else
#define COMPILER_ERROR_PREFIX "error in %s(%u:%u) :: func %s(): "
#endif

#define COMPILER_RAISE(type, function_name, file_name, line, row, message_format, ...)                                                          \
    do {                                                                                                                                        \
        str_format_data vars[] = { (str_format_data) 0, (str_format_data) file_name, line, row, (str_format_data) function_name, __VA_ARGS__ }; \
        u32 vars_length = ARRAY_LENGTH(vars);                                                                                                   \
                                                                                                                                                \
        u32 string_length = 0;                                                                                                                  \
        str_format("%n" COMPILER_ERROR_PREFIX message_format "\n", vars, NULL, &string_length);                                                 \
                                                                                                                                                \
        str output_string = NULL;                                                                                                               \
        RUN_ERROR_CODE_FUNCTION_IGNORE(platform_memory_allocate, (void**) &output_string, sizeof(char) * (string_length + 1));                  \
        str_format("%n" COMPILER_ERROR_PREFIX message_format "\n", vars, output_string, &string_length);                                        \
                                                                                                                                                \
        compiler_raise_error(type, output_string, string_length);                                                                               \
    } while (0) /* TODO: fix usage of RUN_ERROR_CODE_FUNCTION_IGNORE */

#define COMPILER_RAISE_INFO(function_name, file_name, line, row, message_format, ...)    COMPILER_RAISE(COMPILER_MESSAGE_TYPE_INFO,    function_name, file_name, line, row, message_format, __VA_ARGS__)
#define COMPILER_RAISE_WARNING(function_name, file_name, line, row, message_format, ...) COMPILER_RAISE(COMPILER_MESSAGE_TYPE_WARNING, function_name, file_name, line, row, message_format, __VA_ARGS__)
#define COMPILER_RAISE_ERROR(function_name, file_name, line, row, message_format, ...)   COMPILER_RAISE(COMPILER_MESSAGE_TYPE_ERROR,   function_name, file_name, line, row, message_format, __VA_ARGS__)

// Typedefs

typedef enum {
    COMPILER_MESSAGE_TYPE_INFO,
    COMPILER_MESSAGE_TYPE_WARNING,
    COMPILER_MESSAGE_TYPE_ERROR
} COMPILER_MESSAGE_TPE;
typedef byte compiler_message_type; // COMPILER_MESSAGE_TPE

typedef error_code (*wave_compiler_message_function)(compiler_message_type type, str string, u32 length);

// Functions

void compiler_raise_error(compiler_message_type type, str message, u32 message_length);
bool compiler_has_error(void);

error_code wave_compile_bytecode(wave_vm* vm, str source, wave_compiler_message_function message_function);

#endif
