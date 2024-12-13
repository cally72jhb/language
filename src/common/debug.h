#ifndef STANDARD_LIBRARY_DEBUG
#define STANDARD_LIBRARY_DEBUG

// Includes

#include "platform.h"

#include "common/constants.h"
#include "common/defines.h"
#include "common/error_codes.h"
#include "common/macros.h"

#include "common/data/string/string.h"

// Functions

error_code debug_create();
error_code debug_destroy();

error_code debug_builtin_get_buffer(byte** out_buffer, umax size);

// Defines

#if PROGRAM_FEATURE_DEBUG_MODE != 0
    #define CURRENT_FUNCTION_NAME __FUNCTION__

    #ifndef SOURCE_PATH_LENGTH
        #error "[ERROR] SOURCE_PATH_LENGTH debug macro is undefined"
        #define SOURCE_PATH_LENGTH (0)
        #define CURRENT_FILE_PATH (__FILE__)
    #else
        #define CURRENT_FILE_PATH (&((__FILE__)[SOURCE_PATH_LENGTH]))
    #endif

    #define CURRENT_FILE_LINE __LINE__
#endif

// Console Color Codes

#if PROGRAM_FEATURE_NO_CONSOLE_COLORS == 0
    #define ANSI_COLOR_RESET "\x1b[0m"

    // Foreground Colors (Regular & Normal)

    #define ANSI_COLOR_REG_BLK "\x1b[0;30m"
    #define ANSI_COLOR_REG_RED "\x1b[0;31m"
    #define ANSI_COLOR_REG_GRN "\x1b[0;32m"
    #define ANSI_COLOR_REG_YEL "\x1b[0;33m"
    #define ANSI_COLOR_REG_BLU "\x1b[0;34m"
    #define ANSI_COLOR_REG_MAG "\x1b[0;35m"
    #define ANSI_COLOR_REG_CYN "\x1b[0;36m"
    #define ANSI_COLOR_REG_WHT "\x1b[0;37m"

    #define ANSI_COLOR_NRM_BLK "\x1b[0;90m"
    #define ANSI_COLOR_NRM_RED "\x1b[0;91m"
    #define ANSI_COLOR_NRM_GRN "\x1b[0;92m"
    #define ANSI_COLOR_NRM_YEL "\x1b[0;93m"
    #define ANSI_COLOR_NRM_BLU "\x1b[0;94m"
    #define ANSI_COLOR_NRM_MAG "\x1b[0;95m"
    #define ANSI_COLOR_NRM_CYN "\x1b[0;96m"
    #define ANSI_COLOR_NRM_WHT "\x1b[0;97m"

    // Background Colors (Regular & Normal)

    #define ANSI_COLOR_REG_BG_BLK "\x1b[40m"
    #define ANSI_COLOR_REG_BG_RED "\x1b[41m"
    #define ANSI_COLOR_REG_BG_GRN "\x1b[42m"
    #define ANSI_COLOR_REG_BG_YEL "\x1b[43m"
    #define ANSI_COLOR_REG_BG_BLU "\x1b[44m"
    #define ANSI_COLOR_REG_BG_MAG "\x1b[45m"
    #define ANSI_COLOR_REG_BG_CYN "\x1b[46m"
    #define ANSI_COLOR_REG_BG_WHT "\x1b[47m"

    #define ANSI_COLOR_NRM_BG_BLK "\x1b[0;100m"
    #define ANSI_COLOR_NRM_BG_RED "\x1b[0;101m"
    #define ANSI_COLOR_NRM_BG_GRN "\x1b[0;102m"
    #define ANSI_COLOR_NRM_BG_YEL "\x1b[0;103m"
    #define ANSI_COLOR_NRM_BG_BLU "\x1b[0;104m"
    #define ANSI_COLOR_NRM_BG_MAG "\x1b[0;105m"
    #define ANSI_COLOR_NRM_BG_CYN "\x1b[0;106m"
    #define ANSI_COLOR_NRM_BG_WHT "\x1b[0;107m"
#else
    #define ANSI_COLOR_RESET ""

    #define ANSI_COLOR_REG_BLK ""
    #define ANSI_COLOR_REG_RED ""
    #define ANSI_COLOR_REG_GRN ""
    #define ANSI_COLOR_REG_YEL ""
    #define ANSI_COLOR_REG_BLU ""
    #define ANSI_COLOR_REG_MAG ""
    #define ANSI_COLOR_REG_CYN ""
    #define ANSI_COLOR_REG_WHT ""

    #define ANSI_COLOR_NRM_BLK ""
    #define ANSI_COLOR_NRM_RED ""
    #define ANSI_COLOR_NRM_GRN ""
    #define ANSI_COLOR_NRM_YEL ""
    #define ANSI_COLOR_NRM_BLU ""
    #define ANSI_COLOR_NRM_MAG ""
    #define ANSI_COLOR_NRM_CYN ""
    #define ANSI_COLOR_NRM_WHT ""

    #define ANSI_COLOR_REG_BG_BLK ""
    #define ANSI_COLOR_REG_BG_RED ""
    #define ANSI_COLOR_REG_BG_GRN ""
    #define ANSI_COLOR_REG_BG_YEL ""
    #define ANSI_COLOR_REG_BG_BLU ""
    #define ANSI_COLOR_REG_BG_MAG ""
    #define ANSI_COLOR_REG_BG_CYN ""
    #define ANSI_COLOR_REG_BG_WHT ""

    #define ANSI_COLOR_NRM_BG_BLK ""
    #define ANSI_COLOR_NRM_BG_RED ""
    #define ANSI_COLOR_NRM_BG_GRN ""
    #define ANSI_COLOR_NRM_BG_YEL ""
    #define ANSI_COLOR_NRM_BG_BLU ""
    #define ANSI_COLOR_NRM_BG_MAG ""
    #define ANSI_COLOR_NRM_BG_CYN ""
    #define ANSI_COLOR_NRM_BG_WHT ""
#endif

// Debug Macros

#if PROGRAM_FEATURE_DEBUG_MODE != 0

    // Debugging Prefixes & Suffix

    #if PROGRAM_FEATURE_NO_CONSOLE_COLORS == 0
        #define DEBUG_FILE_FORMAT_PREFIX ANSI_COLOR_NRM_BLK " %s(%u): " ANSI_COLOR_RESET

        #define DEBUG_INFO_PREFIX    ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_BLU "INFO"    ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET
        #define DEBUG_WARNING_PREFIX ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_YEL "WARNING" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET
        #define DEBUG_ERROR_PREFIX   ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_RED "ERROR"   ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET
        #define DEBUG_FATAL_PREFIX   ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_RED "FATAL"   ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET

        #define DEBUG_STACKTRACE_PREFIX          ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_GRN "STCKTRCE" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET DEBUG_FILE_FORMAT_PREFIX ANSI_COLOR_NRM_BLK "error calling " ANSI_COLOR_NRM_CYN "%s(%s)" ANSI_COLOR_NRM_BLK ": " ANSI_COLOR_RESET
        #define DEBUG_FUNCTION_EXECUTION_PREFIX  ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_MAG "EXECUTE"  ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET DEBUG_FILE_FORMAT_PREFIX ANSI_COLOR_NRM_BLK "calling "       ANSI_COLOR_NRM_CYN "%s(%s)" ANSI_COLOR_NRM_BLK ";"  ANSI_COLOR_RESET
    #else
        #define DEBUG_FILE_FORMAT_PREFIX " %s(%i): "

        #define DEBUG_INFO_PREFIX "[INFO]"
        #define DEBUG_WARNING_PREFIX "[WARNING]"
        #define DEBUG_ERROR_PREFIX "[ERROR]"
        #define DEBUG_FATAL_PREFIX "[FATAL]"

        #define DEBUG_STACKTRACE_PREFIX          "[STCKTRCE] " DEBUG_FILE_FORMAT_PREFIX "error calling %s(%s): "
        #define DEBUG_FUNCTION_EXECUTION_PREFIX  "[EXECUTE]"   DEBUG_FILE_FORMAT_PREFIX "called %s(%s);"
    #endif

    #if PROGRAM_FEATURE_NO_CONSOLE_COLORS == 0
        #define DEBUG_SUFFIX ANSI_COLOR_RESET
    #else
        #define DEBUG_SUFFIX ""
    #endif

    // Debug Implementation

    #define DEBUG_FORMAT(format, ...)                                                                                               \
        do {                                                                                                                        \
            str_format_data vars[] = { (str_format_data) 0, __VA_ARGS__ };                                                          \
                                                                                                                                    \
            u32 string_length = 0;                                                                                                  \
            str_format("%n" format, vars, NULL, &string_length);                                                                    \
                                                                                                                                    \
            str output_string = NULL;                                                                                               \
            RUN_ERROR_CODE_FUNCTION_IGNORE(debug_builtin_get_buffer, (byte**) &output_string, sizeof(char) * (string_length + 1));  \
            str_format("%n" format, vars, output_string, &string_length);                                                           \
                                                                                                                                    \
            RUN_ERROR_CODE_FUNCTION_IGNORE(platform_commandline_print, output_string, string_length);                               \
        } while (0)

    #define DEBUG_RAW(string)           do { RUN_ERROR_CODE_FUNCTION(          platform_commandline_print, string, ARRAY_LENGTH(string)); } while (0)
    #define DEBUG_RAW_TRACELESS(string) do { RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, string, ARRAY_LENGTH(string)); } while (0)

    // Debugging

    #if PROGRAM_FEATURE_DEBUG_STACK_TRACE != 0
        #define DEBUG_TYPE(type, format, ...) DEBUG_FORMAT(type DEBUG_FILE_FORMAT_PREFIX format DEBUG_SUFFIX "\n", (str_format_data) CURRENT_FILE_PATH, (str_format_data) CURRENT_FILE_LINE, __VA_ARGS__)
    #else
        #define DEBUG_TYPE(type, format, ...) DEBUG_FORMAT(type " " format DEBUG_SUFFIX "\n", __VA_ARGS__)
    #endif

    // Different Debugging Types

    #define DEBUG_INFO(format, ...)    DEBUG_TYPE(DEBUG_INFO_PREFIX,      format, __VA_ARGS__)
    #define DEBUG_WARNING(format, ...) DEBUG_TYPE(DEBUG_WARNING_PREFIX,   format, __VA_ARGS__)
    #define DEBUG_ERROR(format, ...)   DEBUG_TYPE(DEBUG_ERROR_PREFIX,     format, __VA_ARGS__)
    #define DEBUG_FATAL(format, ...)   DEBUG_TYPE(DEBUG_FATAL_PREFIX,     format, __VA_ARGS__)

    #define DEBUG_NEW_LINE() DEBUG_FORMAT("\n")

    #define DEBUG_STACKTRACE(function_name, code, ...)   DEBUG_FORMAT(DEBUG_STACKTRACE_PREFIX         "%s\n", (str_format_data) CURRENT_FILE_PATH, (str_format_data) CURRENT_FILE_LINE, (str_format_data) #function_name, (str_format_data) #__VA_ARGS__, (str_format_data) error_codes_get_error_code_name(code))
    #define DEBUG_FUNCTION_EXECUTION(function_name, ...) DEBUG_FORMAT(DEBUG_FUNCTION_EXECUTION_PREFIX "\n",   (str_format_data) CURRENT_FILE_PATH, (str_format_data) CURRENT_FILE_LINE, (str_format_data) #function_name, (str_format_data) #__VA_ARGS__)

    #define DEBUG_ASSERT(predicate, message)                                                                            \
        do {                                                                                                            \
            if (!(predicate)) {                                                                                         \
                DEBUG_WARNING("assertion failed (%s): %s", (str_format_data) #predicate, (str_format_data) message);    \
            }                                                                                                           \
        } while (0)

#else

    // Debugging Disabled

    #define DEBUG_FORMAT(...) EMPTY_CODE_BLOCK()

    #define DEBUG_RAW(...) EMPTY_CODE_BLOCK()
    #define DEBUG_RAW_TRACELESS(...) EMPTY_CODE_BLOCK()

    #define DEBUG_TYPE(...) EMPTY_CODE_BLOCK()

    #define DEBUG_INFO(...)    EMPTY_CODE_BLOCK()
    #define DEBUG_WARNING(...) EMPTY_CODE_BLOCK()
    #define DEBUG_ERROR(...)   EMPTY_CODE_BLOCK()
    #define DEBUG_FATAL(...)   EMPTY_CODE_BLOCK()

    #define DEBUG_NEW_LINE(...) EMPTY_CODE_BLOCK()

    #define DEBUG_STACKTRACE(...) EMPTY_CODE_BLOCK()
    #define DEBUG_FUNCTION_EXECUTION(...) EMPTY_CODE_BLOCK()

    #define DEBUG_ASSERT(...) EMPTY_CODE_BLOCK()

#endif

#endif
