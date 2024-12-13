#include "main.h"

#include "common/debug.h"

#include "language/compiler/wave_compiler.h"
#include "language/compiler/wave_disassembler.h"

#include "language/runtime/wave_vm.h"
#include "language/runtime/wave_vm_container.h"

// Program Main Functions

error_code program_startup(void) {
    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_memory_heap_initialize); // no memory allocation calls should be made before this statement

    #if PROGRAM_FEATURE_DEBUG_MODE != 0
    RUN_ERROR_CODE_FUNCTION_TRACELESS(debug_create); // no debugging should happen before this statement
    #endif

    error_code result_platform_create_commandline = platform_commandline_create();
    if (result_platform_create_commandline != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        RUN_ERROR_CODE_FUNCTION_IGNORE_FLAG(platform_commandline_destroy, ERROR_FLAG_WARNING);
        #if PROGRAM_FEATURE_DEBUG_MODE == 0
        return result_platform_create_commandline;
        #endif
    }

    DEBUG_INFO("startup successful");

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code program_shutdown_request(void) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code program_cleanup(void) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code program_close(void) {
    RUN_ERROR_CODE_FUNCTION(platform_commandline_shutdown);
    #if PROGRAM_FEATURE_DEBUG_MODE != 0
    RUN_ERROR_CODE_FUNCTION_TRACELESS(debug_destroy);
    #endif

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#define PRINT_STRING(string) RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, string, STRING_LENGTH(string))

static error_code builtin_compiler_message(compiler_message_type type, str string, u32 length) {
    switch (type) {
        case COMPILER_MESSAGE_TYPE_INFO: {
            PRINT_STRING(ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_CYN "COMPILER_INFO" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET " ");
            break;
        }

        case COMPILER_MESSAGE_TYPE_WARNING: {
            PRINT_STRING(ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_YEL "COMPILER_WARNING" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET " ");
            break;
        }

        case COMPILER_MESSAGE_TYPE_ERROR:
        default: {
            PRINT_STRING(ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_RED "COMPILER_ERROR" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET " ");
            break;
        }
    }

    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, string, length);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

static error_code builtin_disassembler_print(str string, u32 length) {
    PRINT_STRING(ANSI_COLOR_NRM_BLK "[" ANSI_COLOR_NRM_YEL "DISASSEMBLER" ANSI_COLOR_NRM_BLK "]" ANSI_COLOR_RESET " ");
    RUN_ERROR_CODE_FUNCTION_TRACELESS(platform_commandline_print, string, length);
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code program_main(void) {
    DEBUG_NEW_LINE();

    // read bytecode code from file

    DEBUG_INFO("reading file...");

    str file_path = "../resources/scripts/source.wave";

    u32 file_length = 0;
    RUN_ERROR_CODE_FUNCTION(platform_get_file_content_length, file_path, &file_length);
    u32 file_read_length = 0;
    str file_content = NULL;
    RUN_ERROR_CODE_FUNCTION(platform_memory_allocate_clear, (void**) &file_content, file_length + 1);
    RUN_ERROR_CODE_FUNCTION(platform_read_file_length, file_path, file_length, &file_content, &file_read_length);
    if (file_length != file_read_length) {
        return ERROR_CODE_NOT_IMPLEMENTED;
    }

    // init vm

    DEBUG_INFO("initializing virtual machine...");

    wave_vm vm;
    RUN_ERROR_CODE_FUNCTION(wave_vm_initialize, &vm, platform_memory_allocate, platform_memory_allocate_clear, platform_memory_reallocate, platform_memory_deallocate);
    wave_vm_set_stack_sizes(&vm, WAVE_VM_INIT_DEFAULT_PARAMETERS);
    RUN_ERROR_CODE_FUNCTION(wave_vm_register_default_functions, &vm);
    RUN_ERROR_CODE_FUNCTION(wave_vm_function_registration_done, &vm);

    // compile bytecode

    DEBUG_INFO("compiling bytecode...");
    error_code result_compile = wave_compile_bytecode(&vm, file_content, builtin_compiler_message);
    if (result_compile != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        #if PROGRAM_FEATURE_DEBUG_MODE != 0
        DEBUG_INFO("disassembling bytecode:");
        RUN_ERROR_CODE_FUNCTION(wave_disassemble, &vm, builtin_disassembler_print);
        #endif

        RUN_ERROR_CODE_FUNCTION(wave_vm_destroy, &vm);
        return result_compile;
    }

    DEBUG_INFO("bytecode compiled");

    #if PROGRAM_FEATURE_DEBUG_MODE != 0
    DEBUG_INFO("disassembling bytecode:");
    RUN_ERROR_CODE_FUNCTION(wave_disassemble, &vm, builtin_disassembler_print);
    #endif

    // init runtime

    RUN_ERROR_CODE_FUNCTION(wave_vm_initialize_runtime, &vm, WAVE_VM_INIT_DEFAULT_PARAMETERS);

    // execute bytecode

    RUN_ERROR_CODE_FUNCTION(wave_vm_begin_execution, &vm);

    RUN_ERROR_CODE_FUNCTION(wave_vm_execute_entire_safe, &vm);
    DEBUG_INFO("result: %u64", vm.result.number_value.value_u64);
    DEBUG_NEW_LINE();

    // free bytecode code string and vm

    RUN_ERROR_CODE_FUNCTION(wave_vm_destroy, &vm);

    // shutdown commandline

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
