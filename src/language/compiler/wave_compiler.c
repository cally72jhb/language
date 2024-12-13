#include "wave_compiler.h"

#include "common/constants.h"
#include "common/error_codes.h"
#include "common/debug.h"

#include "common/data/string/string.h"

#include "language/compiler/wave_compiler_common.h"
#include "language/compiler/wave_parser.h"
#include "language/compiler/wave_tokenizer.h"

#include "language/runtime/wave_vm.h"

// Typedefs

typedef struct {
    compiler_message_type type;
    u32 message_length;
    str message;
} compiler_error;

// compilation interface

struct {
    wave_vm* vm;

    // error handling

    u32 error_count;
    u32 error_capacity;
    compiler_error* errors;
} compiler;

// Functions

void compiler_raise_error(compiler_message_type type, str message, u32 message_length) {
    if (compiler.error_count + 1 >= compiler.error_capacity) {
        compiler.error_capacity += 8;
        if (compiler.vm->reallocate_memory((void**) &(compiler.errors), sizeof(compiler_error) * compiler.error_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            return; // don't try to throw another error, it probably won't work
        }
    }

    compiler.errors[compiler.error_count] = (compiler_error) { .type = type, .message_length = message_length, .message = message };
    compiler.error_count++;
}

bool compiler_has_error(void) {
    return compiler.error_count > 0;
}

error_code wave_compile_bytecode(wave_vm* vm, str source, wave_compiler_message_function message_function) {
    const wave_memory_allocation_function allocate_memory = vm->allocate_memory;
    const wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    error_code result = ERROR_CODE_EXECUTION_SUCCESSFUL;

    #define PRINT_STRING(type, string) do { if (message_function != NULL) { RUN_ERROR_CODE_FUNCTION_TRACELESS(message_function, type, string "\n", STRING_LENGTH(string) + 1); } } while (0)

    // initialize compiler

    compiler.errors = NULL;
    compiler.error_capacity = 8;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &compiler.errors, sizeof(compiler_error) * compiler.error_capacity);
    compiler.error_count = 0;

    // tokenize source

    parse_token* token_stack_start = NULL;
    parse_token* token_stack_end = NULL;
    byte* data_stack_start = NULL;
    byte* data_stack_end = NULL;
    if (wave_compiler_tokenize(vm, source, &token_stack_start, &token_stack_end, &data_stack_start, &data_stack_end) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        PRINT_STRING(COMPILER_MESSAGE_TYPE_ERROR, "failed to tokenize source, attempting to deallocate temporary memory...");
        RUN_ERROR_CODE_FUNCTION(wave_compiler_tokenizer_destroy);
        PRINT_STRING(COMPILER_MESSAGE_TYPE_ERROR, "temporary memory deallocated");

        result = ERROR_CODE_EXECUTION_FAILED;
        goto wave_compile_bytecode_print_errors;
    }

    // parse and compile tokens

    if (wave_compiler_parser_compile(vm, token_stack_start, token_stack_end, data_stack_start, data_stack_end) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        PRINT_STRING(COMPILER_MESSAGE_TYPE_ERROR, "failed to parse and compile source, attempting to deallocate temporary memory...");
        RUN_ERROR_CODE_FUNCTION(wave_compiler_parser_destroy);
        PRINT_STRING(COMPILER_MESSAGE_TYPE_ERROR, "temporary memory deallocated");

        result = ERROR_CODE_EXECUTION_FAILED;
        goto wave_compile_bytecode_print_errors;
    }

    // print errors

    wave_compile_bytecode_print_errors: {}

    if (message_function != NULL && compiler_has_error()) {
        for (u32 i = 0; i < compiler.error_count; i++) {
            if (compiler.errors[i].message != NULL) {
                RUN_ERROR_CODE_FUNCTION_TRACELESS(message_function, compiler.errors[i].type, compiler.errors[i].message, compiler.errors[i].message_length);
            }
        }
    }

    #undef PRINT_STRING

    // deallocate temporary memory

    RUN_ERROR_CODE_FUNCTION(wave_compiler_tokenizer_destroy);

    for (u32 i = 0; i < compiler.error_count; i++) {
        if (compiler.errors[i].message != NULL) {
            RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void*) compiler.errors[i].message);
        }
    }

    RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void*) compiler.errors);

    // output result

    vm->bytecode_current = vm->bytecode_start;

    return result;
}
