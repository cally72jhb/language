#if defined(PROGRAM_PLATFORM_CONSTANTS) && PROGRAM_PLATFORM_CONSTANTS != 0
#define PLATFORM_BIT_COUNT (64)
#define PLATFORM_NAME_STRING ("win64console")
#define PLATFORM_MAX_PATH_LENGTH (MAX_PATH)
#else

#if !defined(PROGRAM_PLATFORM_CONSTANTS) || PROGRAM_PLATFORM_CONSTANTS == 0

// Platform Libraries

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>
#include <handleapi.h>

// Include Platform and Main

#include "platform.h"

#include "main.h"

// Includes

#include "common/constants.h"
#include "common/error_codes.h"
#include "common/debug.h"

#include "common/memory/memory.h"

// Globals

static struct platform_globals {
    bool shutting_down;

    HANDLE heap_handle;

    HANDLE commandline_process;
    HANDLE commandline_thread;
    bool extern_commandline;
    bool commandline_allocated;

    HANDLE commandline_input_handle;
    HANDLE commandline_output_handle;

    DWORD commandline_prev_input_mode;
    DWORD commandline_prev_output_mode;
} win64_globals;

// Main Function

int main(int argument_count, char* arguments[]) {
    error_code return_value = ERROR_CODE_EXECUTION_SUCCESSFUL;
    memory_clear(&win64_globals, sizeof(struct platform_globals));

    // parse program arguments

    for (u32 i = 0; i < argument_count; i++) {
        if (str_is_equals(arguments[i], "-extern") || str_is_equals(arguments[i], "-extern-console") || str_is_equals(arguments[i], "-extern_console")) {
            win64_globals.extern_commandline = true;
            break;
        }
    }

    // startup program

    error_code result_startup = program_startup();
    if (result_startup != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        return_value = result_startup;
        goto main_cleanup;
    }

    // execute the main program

    error_code result_main = program_main();
    if (result_main != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        return_value = result_main;
        goto main_cleanup;
    }

    // cleanup everything before returning

    main_cleanup: {
        cstr error_code_name = error_codes_get_error_code_name(return_value);
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, "\nprocess finished with error code: ", STRING_LENGTH("\nprocess finished with error code: "));
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, (str) error_code_name, str_length((str) error_code_name));
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, "\nshutting down...\n\n", STRING_LENGTH("\nshutting down...\n\n"));

        // shot down

        error_code result_cleanup = program_cleanup(); // clean up everything if possible
        error_code result_close = program_close(); // close everything that is possible

        if (return_value != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            return return_value;
        } else {
            return result_cleanup == ERROR_CODE_EXECUTION_SUCCESSFUL ? result_close : result_cleanup;
        }
    }
}

// Platform Functions

// Shutdown Function

error_code platform_shutdown(void) {
    win64_globals.shutting_down = true;
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

bool platform_is_shutting_down(bool* status) {
    *status = win64_globals.shutting_down;
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

// Command Line Functions

BOOL WINAPI platform_builtin_commandline_handler_routine(_In_ DWORD control_type) {
    return TRUE;
}

#define PLATFORM_BUILTIN_PRINT_CONSTANT(string) RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, string, STRING_LENGTH(string))

error_code platform_commandline_create(void) {
    #if PROGRAM_FEATURE_EXTERN_CONSOLE == 0
    if (!win64_globals.extern_commandline) {
        if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) {
            win64_globals.extern_commandline = true;
        } else {
            win64_globals.commandline_process = NULL;
            win64_globals.commandline_thread = NULL;

            win64_globals.extern_commandline = false;
        }
    }
    #endif

    #if PROGRAM_FEATURE_EXTERN_CONSOLE != 0
    {
    #else
    if (win64_globals.extern_commandline) {
    #endif
        STARTUPINFO startup_info;
        PROCESS_INFORMATION process_information;
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_clear, &startup_info, sizeof(STARTUPINFO));
        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_clear, &process_information, sizeof(PROCESS_INFORMATION));
        startup_info.cb = sizeof(STARTUPINFO);

        if (CreateProcessA("C:\\Windows\\System32\\cmd.exe", NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup_info, &process_information) == 0) {
            if (AllocConsole() == 0) {
                return ERROR_CODE_FAILED_TO_CREATE_COMMANDLINE;
            }

            win64_globals.commandline_process = NULL;
            win64_globals.commandline_thread = NULL;

            win64_globals.commandline_allocated = true;

            goto platform_commandline_create_obtain_handles;
        }

        WaitForInputIdle(process_information.hProcess, INFINITE);
        for (u32 i = 0; i < 100; i++) {
            if (AttachConsole(process_information.dwProcessId) == 0) {
                RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_sleep_ms, 10);
            } else {
                break;
            }
        }

        RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_sleep_ms, 100);

        win64_globals.commandline_process = process_information.hProcess;
        win64_globals.commandline_thread = process_information.hThread;
    }

    // obtain input and output handles

    platform_commandline_create_obtain_handles: {}

    HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle == INVALID_HANDLE_VALUE) {
        return ERROR_CODE_FAILED_TO_CREATE_COMMANDLINE;
    }

    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (input_handle == INVALID_HANDLE_VALUE) {
        return ERROR_CODE_FAILED_TO_CREATE_COMMANDLINE;
    }

    // enable console colors

    if (GetConsoleMode(input_handle, &win64_globals.commandline_prev_input_mode) == 0) {
        return ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
    }

    if (GetConsoleMode(output_handle, &win64_globals.commandline_prev_output_mode) == 0) {
        return ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
    }

    if (SetConsoleMode(output_handle, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        return ERROR_CODE_FAILED_TO_CREATE_COMMANDLINE;
    }

    SetConsoleCtrlHandler(platform_builtin_commandline_handler_routine, TRUE);

    win64_globals.commandline_input_handle = input_handle;
    win64_globals.commandline_output_handle = output_handle;

    // set commandline title

    if (win64_globals.extern_commandline) {
        PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[!p");
        PLATFORM_BUILTIN_PRINT_CONSTANT("\033c");
    } else {
        PLATFORM_BUILTIN_PRINT_CONSTANT("\n");
    }

    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b]0;" PROGRAM_IDENTIFIER " - " "debug" "\x07"); // console title
    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[1\x20q"); // blinking block cursor

    PLATFORM_BUILTIN_PRINT_CONSTANT("\ncommandline started successfully..\n\n");

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_commandline_shutdown(void) {
    error_code code = ERROR_CODE_EXECUTION_SUCCESSFUL;

    PLATFORM_BUILTIN_PRINT_CONSTANT("\nprocess closed\npress any key to quit the debugger...\n\n");
#if PROGRAM_FEATURE_NO_CONSOLE_COLORS == 0
    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[93;101m" " EXIT " ANSI_COLOR_RESET " " "\n\x1b[1F\x1b[8G");
#else
    PLATFORM_BUILTIN_PRINT_CONSTANT(" EXIT "                                     "\n\x1b[1F\x1b[8G");
#endif

    if (SetConsoleMode(win64_globals.commandline_input_handle, ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE) == 0) {
        code = ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
        goto try_to_destroy;
    }

    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[1\x20q"); // blinking block cursor
    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[1s"); // save cursor position
    PLATFORM_BUILTIN_PRINT_CONSTANT("\n");
    PLATFORM_BUILTIN_PRINT_CONSTANT("\x1b[1u"); // load cursor position

    DWORD events_read = 0;
    INPUT_RECORD input_buffer[1] = { 0 };
    while (true) {
        if (ReadConsoleInputA(win64_globals.commandline_input_handle, input_buffer, ARRAY_LENGTH(input_buffer), &events_read) == 0) {
            code = ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
            goto try_to_destroy;
        }

        if (input_buffer[0].EventType == KEY_EVENT) {
            FlushConsoleInputBuffer(win64_globals.commandline_input_handle);
            break;
        }
    }

    // reset console modes

    if (!win64_globals.extern_commandline) {
        if (SetConsoleMode(win64_globals.commandline_input_handle, win64_globals.commandline_prev_input_mode) == 0) {
            code = ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
            goto try_to_destroy;
        }

        if (SetConsoleMode(win64_globals.commandline_output_handle, win64_globals.commandline_prev_output_mode) == 0) {
            code = ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
            goto try_to_destroy;
        }
    }

    // destroy the commandline

    try_to_destroy: {}

    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_destroy);

    return code;
}

error_code platform_commandline_destroy(void) {
    win64_globals.commandline_input_handle = INVALID_HANDLE_VALUE;
    win64_globals.commandline_output_handle = INVALID_HANDLE_VALUE;

    // free the console

    if (FreeConsole() == 0) {
        return ERROR_CODE_FAILED_TO_DESTROY_COMMANDLINE;
    }

    // terminate the console

    if (!win64_globals.commandline_allocated && win64_globals.commandline_process != NULL && win64_globals.commandline_thread != NULL) {
        DWORD exit_code = 0;
        if (GetExitCodeProcess(win64_globals.commandline_process, (LPDWORD) &exit_code) == 0) {
            exit_code = 0;
        }

        if (TerminateProcess(win64_globals.commandline_process, exit_code) == 0) {
            return ERROR_CODE_FAILED_TO_CREATE_COMMANDLINE;
        }

        WaitForInputIdle(win64_globals.commandline_process, INFINITE);

        // close the console process handles

        if (CloseHandle(win64_globals.commandline_process) == 0) {
            return ERROR_CODE_WIN64_FAILED_CLOSE_HANDLE;
        }

        if (CloseHandle(win64_globals.commandline_thread) == 0) {
            return ERROR_CODE_WIN64_FAILED_CLOSE_HANDLE;
        }

        win64_globals.commandline_process = NULL;
        win64_globals.commandline_thread = NULL;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_commandline_print(str string, u32 length) {
    if (win64_globals.commandline_output_handle == INVALID_HANDLE_VALUE) {
        #if PROGRAM_FEATURE_DEBUG_MODE != 0
        return ERROR_CODE_EXECUTION_SUCCESSFUL; // if debugging is enabled and the editors builtin debugger is enabled, this function will probably fail, so don't care about these errors
        #else
        return ERROR_CODE_FAILED_TO_COMMANDLINE_PRINT;
        #endif
    }

    DWORD written_length = 0;
    if (WriteConsoleA(win64_globals.commandline_output_handle, string, length, &written_length, NULL) == 0) {
        return ERROR_CODE_FAILED_TO_COMMANDLINE_PRINT;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#undef PLATFORM_BUILTIN_PRINT_CONSTANT

// Other Functions

error_code platform_sleep_ms(u64 milliseconds) {
    Sleep((DWORD) milliseconds);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

// Time Functions

#define UNIX_TIME_START (0x019DB1DED53E8000) /* January 1, 1970 (start of Unix epoch) in "ticks" */
#define TICKS_PER_SECOND (10000) /* a tick is 100ns */

error_code platform_get_time_ms(u64* out_time) {
    FILETIME file_time;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &file_time, sizeof(FILETIME));
    GetSystemTimeAsFileTime(&file_time);

    LARGE_INTEGER time;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &time, sizeof(LARGE_INTEGER));
    time.LowPart = file_time.dwLowDateTime;
    time.HighPart = (LONG) file_time.dwHighDateTime;

    *out_time = (u64) ((((u64) time.QuadPart) - UNIX_TIME_START) / TICKS_PER_SECOND);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_get_system_time_ms(u64* out_time) {
    SYSTEMTIME local_time;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &local_time, sizeof(SYSTEMTIME));
    GetLocalTime(&local_time);

    FILETIME file_time;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &file_time, sizeof(FILETIME));
    SystemTimeToFileTime(&local_time, &file_time);

    LARGE_INTEGER time;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &time, sizeof(LARGE_INTEGER));
    time.LowPart = file_time.dwLowDateTime;
    time.HighPart = (LONG) file_time.dwHighDateTime;

    *out_time = (u64) ((((u64) time.QuadPart) - UNIX_TIME_START) / TICKS_PER_SECOND);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#undef UNIX_TIME_START
#undef UNIX_TIME_START

// System Functions

error_code platform_get_available_memory_size(umax* out_size) {
    MEMORYSTATUSEX status;
    RUN_ERROR_CODE_FUNCTION(platform_memory_clear, &status, sizeof(MEMORYSTATUSEX));

    status.dwLength = sizeof(status);

    if (GlobalMemoryStatusEx(&status) == 0) {
        return ERROR_CODE_FAILED_TO_RETRIEVE_MEMORY_SIZE;
    }

    *out_size = (umax) status.ullAvailPageFile;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

// Memory Functions

error_code platform_memory_heap_initialize() {
    win64_globals.heap_handle = GetProcessHeap();
    if (win64_globals.heap_handle == NULL) {
        return ERROR_CODE_WIN64_FAILED_TO_RETRIEVE_HEAP_HANDLE;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_allocate(void** out_pointer, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (out_pointer == NULL) {
        return ERROR_CODE_FAILED_TO_ALLOCATE;
    }
#endif

    void* result = HeapAlloc(win64_globals.heap_handle, 0, size);
    if (result == NULL) {
        return ERROR_CODE_FAILED_TO_ALLOCATE;
    }

    *out_pointer = result;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_allocate_clear(void** out_pointer, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (out_pointer == NULL) {
        return ERROR_CODE_FAILED_TO_ALLOCATE;
    }
#endif

    void* result = HeapAlloc(win64_globals.heap_handle, HEAP_ZERO_MEMORY, size);
    if (result == NULL) {
        return ERROR_CODE_FAILED_TO_ALLOCATE;
    }

    *out_pointer = result;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_reallocate(void** in_out_pointer, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (in_out_pointer == NULL) {
        return ERROR_CODE_FAILED_TO_RE_ALLOCATE;
    }
#endif

    void* result = HeapReAlloc(win64_globals.heap_handle, 0, *in_out_pointer, size);
    if (result == NULL) {
        return ERROR_CODE_FAILED_TO_RE_ALLOCATE;
    }

    *in_out_pointer = result;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_deallocate(void* pointer) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (pointer == NULL) {
        return ERROR_CODE_FAILED_TO_FREE;
    }
#endif

    if (HeapFree(win64_globals.heap_handle, 0, pointer) == 0) {
        return ERROR_CODE_FAILED_TO_FREE;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_set(void* destination, u32 value, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (destination == NULL) {
        return ERROR_CODE_FAILED_TO_MEMORY_SET;
    }
#endif

    memory_set_bit_32(destination, value, size); // TODO: does memset always use the first byte while copying the tail?

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_clear(void* destination, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (destination == NULL) {
        return ERROR_CODE_FAILED_TO_MEMORY_SET;
    }
#endif

    memory_clear(destination, size);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_memory_copy(void* source, void* destination, umax size) {
#if PROGRAM_FEATURE_SAFE_MEMORY == 1
    if (source == NULL || destination == NULL) {
        return ERROR_CODE_FAILED_TO_MEMORY_COPY;
    }
#endif

    memory_copy(source, destination, size);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

// File Functions

error_code platform_folder_exists(str path, bool* out_exists) {
    DWORD attributes = GetFileAttributesA(path);

    *out_exists = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_create_folder(str path) {
    bool folder_exists = true;
    RUN_ERROR_CODE_FUNCTION(platform_folder_exists, path, &folder_exists);
    if (folder_exists == true) {
        return ERROR_CODE_EXECUTION_SUCCESSFUL;
    }

    if (CreateDirectoryA(path, NULL) == 0) {
        return ERROR_CODE_FAILED_TO_CREATE_FOLDER;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_file_exists(str path, bool* out_exists) {
    DWORD attributes = GetFileAttributesA(path);

    *out_exists = (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}


error_code platform_create_file(str path, str content) {
    bool file_exists = true;
    RUN_ERROR_CODE_FUNCTION(platform_file_exists, path, &file_exists);
    if (file_exists == true) {
        return ERROR_CODE_FAILED_TO_CREATE_FILE;
    }

    HANDLE file_handle = CreateFileA((LPCSTR) path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        return ERROR_CODE_FAILED_TO_CREATE_FILE;
    }

    DWORD content_bytes_to_write = (DWORD) str_length(content);
    DWORD bytes_written = 0;
    BOOL result_write_file = WriteFile(file_handle, content, content_bytes_to_write, &bytes_written, NULL);
    if (result_write_file == FALSE || content_bytes_to_write != bytes_written) {
        return ERROR_CODE_FAILED_TO_WRITE_FILE;
    }

    if (CloseHandle(file_handle) == 0) {
        return ERROR_CODE_WIN64_FAILED_CLOSE_HANDLE;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_get_file_content_length(str path, u32* out_length) {
    bool file_exists = false;
    RUN_ERROR_CODE_FUNCTION(platform_file_exists, path, &file_exists);
    if (file_exists == false) {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    HANDLE file_handle = CreateFileA((LPCSTR) path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    DWORD file_size = GetFileSize(file_handle, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    *out_length = (u32) file_size;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code platform_read_file_length(str path, u32 length, str* in_out_content, u32* out_length) {
    bool file_exists = false;
    RUN_ERROR_CODE_FUNCTION(platform_file_exists, path, &file_exists);
    if (file_exists == false) {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    HANDLE file_handle = CreateFileA((LPCSTR) path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    DWORD bytes_read = 0;
    if (ReadFile(file_handle, (LPVOID) *in_out_content, (DWORD) length, &bytes_read, NULL) == FALSE) {
        if (CloseHandle(file_handle) == 0) {
            return ERROR_CODE_WIN64_FAILED_CLOSE_HANDLE;
        }

        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    if (bytes_read > 0 && bytes_read <= length) {
        (*in_out_content)[length] = '\0';
    } else if (bytes_read == 0) {
        (*in_out_content)[0] = '\0';
    } else {
        return ERROR_CODE_FAILED_TO_READ_FILE;
    }

    *out_length = (u32) bytes_read;

    if (CloseHandle(file_handle) == 0) {
        return ERROR_CODE_WIN64_FAILED_CLOSE_HANDLE;
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#endif
#endif
