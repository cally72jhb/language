#ifndef STANDARD_LIBRARY_PLATFORM
#define STANDARD_LIBRARY_PLATFORM

// Platform Defines

#define PROGRAM_PLATFORM_CONSTANTS (1)
#ifdef PROGRAM_PLATFORM_WIN64
    #include "win64console.c"
#endif
#ifdef PROGRAM_PLATFORM_UNIX
    #include "unix.c"
#endif
#undef PROGRAM_PLATFORM_CONSTANTS

#ifndef PLATFORM_BIT_COUNT
    #define PLATFORM_BIT_COUNT (0)
    #error("[ERROR] platform-constant: PLATFORM_BIT_COUNT is undefined")
#endif
#ifndef PLATFORM_NAME_STRING
    #define PLATFORM_NAME_STRING "undefined"
    #error("[ERROR] platform-constant: PLATFORM_NAME_STRING is undefined")
#endif
#ifndef PLATFORM_MAX_PATH_LENGTH
    #define PLATFORM_MAX_PATH_LENGTH (0)
    #error("[ERROR] platform-constant: PLATFORM_MAX_PATH_LENGTH is undefined")
#endif

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

// Shutdown Function

error_code platform_shutdown(void);
bool platform_is_shutting_down(bool* status);

// Command Line Functions

error_code platform_commandline_create(void);
error_code platform_commandline_shutdown(void);
error_code platform_commandline_destroy(void);
error_code platform_commandline_print(str string, u32 length);

// Other Functions

error_code platform_sleep_ms(u64 milliseconds);

// Time Functions

error_code platform_get_time_ms(u64* out_time);
error_code platform_get_system_time_ms(u64* out_time);

// System Functions

error_code platform_get_available_memory_size(umax* out_size);

// Memory Functions

error_code platform_memory_heap_initialize();

error_code platform_memory_allocate(void** out_pointer, umax size);
error_code platform_memory_allocate_clear(void** out_pointer, umax size);
error_code platform_memory_reallocate(void** in_out_pointer, umax size);
error_code platform_memory_deallocate(void* pointer);

error_code platform_memory_set(void* destination, u32 value, umax size);
error_code platform_memory_clear(void* destination, umax size);
error_code platform_memory_copy(void* source, void* destination, umax size);

// File Functions

error_code platform_folder_exists(str path, bool* out_exists);
error_code platform_create_folder(str path);
error_code platform_file_exists(str path, bool* out_exists);
error_code platform_create_file(str path, str content);
error_code platform_get_file_content_length(str path, u32* out_length);
error_code platform_read_file_length(str path, u32 length, str* in_out_content, u32* out_length);

#endif
