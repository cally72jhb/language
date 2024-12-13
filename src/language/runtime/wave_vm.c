#include "wave_vm.h"

#include "common/constants.h"
#include "common/error_codes.h"

#include "common/data/string/hash.h"

#include "language/wave_limits.h"
#include "language/wave_opcodes.h"

// Functions

error_code wave_vm_initialize(
    wave_vm* out_vm,
    wave_memory_allocation_function allocate_memory, wave_memory_allocation_zero_function allocate_zero_memory, wave_memory_reallocation_function reallocate_memory, wave_memory_deallocation_function deallocate_memory
) {
    wave_vm vm = (wave_vm) {
        .allocate_memory = allocate_memory,
        .allocate_zero_memory = allocate_zero_memory,
        .reallocate_memory = reallocate_memory,
        .deallocate_memory = deallocate_memory,

        .bytecode_start = NULL,
        .bytecode_end = NULL,
        .bytecode_current = NULL,

        .exposed_functions = NULL,

        .native_functions = NULL,
        .native_function_callbacks = NULL,
        .function_stack_length = 0,
        .function_stack_element = 0,
        .function_hash = 0,

        .error_branch_offset = 0,
        .error_stack_start = NULL,
        .error_stack_end = NULL,
        .error_stack_top = NULL,
        .error_stack_size = U32_MAX,

        .stack_start = NULL,
        .stack_end = NULL,
        .stack_top = NULL,
        .stack_size = U32_MAX,

        .call_stack_start = NULL,
        .call_stack_end = NULL,
        .call_stack_top = NULL,
        .call_stack_size = U32_MAX,

        .globals_start = NULL,
        .globals_length = 0,
        .globals_size = U32_MAX,

        .execution_finished = false,
        .result = (number) { .number_type = NUMBER_TYPE_U64, .number_value = (union_number) { .value_u64 = 0 } }
    };

    // allocating function stack

    vm.function_stack_length = 64;

    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &vm.native_functions, sizeof(wave_native_function) * vm.function_stack_length);
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &vm.native_function_callbacks, sizeof(wave_native_function_callback) * vm.function_stack_length);

    // output vm

    *out_vm = vm;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

void wave_vm_set_stack_sizes(wave_vm* vm, u32 error_stack_size, u32 stack_size, u32 call_stack_size, u32 globals_size) {
    vm->error_stack_size = error_stack_size;
    vm->stack_size = stack_size;
    vm->call_stack_size = call_stack_size;
    vm->globals_size = globals_size;
}

error_code wave_vm_register_function(wave_vm* vm, wave_native_function function) {
    if (vm->function_stack_element >= WAVE_LIMIT_OPCODE_CALL_NATIVE_MAX) {
        return ERROR_CODE_LANGUAGE_TOO_MANY_NATIVE_FUNCTIONS_DEFINED;
    } else if (vm->function_stack_element >= vm->function_stack_length) {
        const wave_memory_reallocation_function reallocate_memory = vm->reallocate_memory;

        vm->function_stack_length += 32;
        RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(vm->native_functions), sizeof(wave_native_function) * vm->function_stack_length);
        RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(vm->native_function_callbacks), sizeof(wave_native_function_callback) * vm->function_stack_length);
    }

    vm->function_hash ^= function.function_data.name;

    vm->native_functions[vm->function_stack_element] = function;
    vm->native_function_callbacks[vm->function_stack_element] = function.callback;

    vm->function_stack_element++;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_function_registration_done(wave_vm* vm) {
    if (vm->function_stack_element < vm->function_stack_length - 1) {
        const wave_memory_reallocation_function reallocate_memory = vm->reallocate_memory;

        vm->function_stack_length = vm->function_stack_element;
        RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(vm->native_functions), sizeof(wave_native_function) * vm->function_stack_length);
        RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(vm->native_function_callbacks), sizeof(wave_native_function_callback) * vm->function_stack_length);
    }

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_initialize_runtime(wave_vm* vm, u32 error_stack_size, u32 stack_size, u32 call_stack_size, u32 globals_size) {
    const wave_memory_allocation_function allocate_memory = vm->allocate_memory;
    const wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    // deallocate additional native function data, as it isn't needed anymore

    if (vm->native_functions != NULL) {
        RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void *) vm->native_functions);
        vm->native_functions = NULL;
    }

    error_stack_size = vm->error_stack_size != U32_MAX ? vm->error_stack_size : error_stack_size;
    stack_size       = vm->stack_size       != U32_MAX ? vm->stack_size       : stack_size;
    call_stack_size  = vm->call_stack_size  != U32_MAX ? vm->call_stack_size  : call_stack_size;
    globals_size     = vm->globals_size     != U32_MAX ? vm->globals_size     : globals_size;

    // allocating error stack

    error_code* error_stack_start = NULL;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &error_stack_start, sizeof(error_code) * error_stack_size);

    vm->error_stack_start = error_stack_start;
    vm->error_stack_end = error_stack_start + error_stack_size;
    vm->error_stack_top = error_stack_start;

    // allocating stack

    byte* stack_start = NULL;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &stack_start, sizeof(byte) * stack_size);

    vm->stack_start = stack_start;
    vm->stack_end = stack_start + stack_size;
    vm->stack_top = stack_start;

    // allocating call stack

    u32* call_stack_start = NULL;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &call_stack_start, sizeof(u32) * call_stack_size);

    vm->call_stack_start = call_stack_start;
    vm->call_stack_end = call_stack_start + call_stack_size;
    vm->call_stack_top = call_stack_start;

    // allocating globals

    byte* globals_start = NULL;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &globals_start, sizeof(u32) * globals_size);

    vm->globals_start = globals_start;
    vm->globals_length = globals_size;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_begin_execution(wave_vm* vm) {
    if (vm->bytecode_end - vm->bytecode_start < sizeof(string_hash)) {
        return ERROR_CODE_LANGUAGE_RUNTIME_BYTECODE_MISSING_FUNCTION_HASH;
    }

    if (*((string_hash*) vm->bytecode_start) != vm->function_hash) {
        return ERROR_CODE_LANGUAGE_RUNTIME_BYTECODE_FUNCTION_HASH_NOT_MATCHING;
    }

    // read entrypoint branch offset

    u32 entrypoint_branch_offset = *((u32*) (vm->bytecode_start + sizeof(string_hash)));
    u16 parameter_size = *((u16*) (vm->bytecode_start + entrypoint_branch_offset));
    u16 locals_stack_frame_size = *((u16*) (vm->bytecode_start + entrypoint_branch_offset + sizeof(u16)));
    vm->bytecode_current = vm->bytecode_start + entrypoint_branch_offset + sizeof(u16) + sizeof(u16); // move instruction pointer to the first instruction

    // reset stacks

    vm->error_stack_top = vm->error_stack_start;
    vm->stack_top = vm->stack_start + parameter_size + locals_stack_frame_size;

    // initialize call stack

    vm->call_stack_start[0] = 0; // parent instruction pointer (undefined)
    vm->call_stack_start[1] = entrypoint_branch_offset; // child instruction pointer (entrypoint branch offset)
    vm->call_stack_start[2] = 0; // stack frame (0 because the stack is empty)

    vm->call_stack_top = vm->call_stack_start + 3;

    // reset other states

    vm->error_branch_offset = 0;

    vm->execution_finished = false;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_begin_function_execution(wave_vm* vm, string_hash function_name) {
    u32 function_branch_offset = 0;

    byte* exposed_functions_bytecode = vm->bytecode_start + sizeof(string_hash) + sizeof(u32);
    u32* exposed_functions = (u32*) (exposed_functions_bytecode + sizeof(u32)); // skip builtin function hash, entrypoint branch offset and exposed function index length
    u32 exposed_function_length = *((u32*) exposed_functions_bytecode); // read exposed function index length

    u32 bound_left = 0;
    u32 bound_right = exposed_function_length - 1;
    while (bound_left <= bound_right) {
        u32 middle_index = (bound_left + bound_right) / 2;
        if (exposed_functions[middle_index * 2] < function_name) {
            bound_left = middle_index + 1;
        } else if (exposed_functions[middle_index * 2] > function_name) {
            bound_right = middle_index - 1;
        } else {
            function_branch_offset = exposed_functions[middle_index * 2 + 1];
            goto wave_vm_begin_function_execution_end;
        }
    }

    return ERROR_CODE_EXECUTION_FAILED; // if we land here, the function is not exposed

    wave_vm_begin_function_execution_end: {}

    // read entrypoint branch offset

    u16 parameter_size = *((u16*) (vm->bytecode_start + function_branch_offset));
    u16 locals_stack_frame_size = *((u16*) (vm->bytecode_start + function_branch_offset + sizeof(u16)));
    vm->bytecode_current = vm->bytecode_start + function_branch_offset + sizeof(u16) + sizeof(u16); // move instruction pointer to the desired function start

    // reset stacks

    vm->error_stack_top = vm->error_stack_start;
    vm->stack_top = vm->stack_start + parameter_size + locals_stack_frame_size;

    // initialize call stack

    vm->call_stack_start[0] = 0; // parent instruction pointer (undefined)
    vm->call_stack_start[1] = function_branch_offset; // child instruction pointer (entrypoint branch offset)
    vm->call_stack_start[2] = 0; // stack frame (0 because the stack is empty)

    vm->call_stack_top = vm->call_stack_start + 3;

    // reset other states

    vm->error_branch_offset = 0;

    vm->execution_finished = false;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_destroy(wave_vm* vm) {
    wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    #define DEALLOCATE_SAFE(pointer) do { if (pointer != NULL) { RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void*) pointer); } } while (0)

    DEALLOCATE_SAFE(vm->native_functions);

    DEALLOCATE_SAFE(vm->native_function_callbacks);
    DEALLOCATE_SAFE(vm->error_stack_start);
    DEALLOCATE_SAFE(vm->stack_start);
    DEALLOCATE_SAFE(vm->call_stack_start);
    DEALLOCATE_SAFE(vm->globals_start);

    DEALLOCATE_SAFE(vm->bytecode_start);

    #undef DEALLOCATE_SAFE

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
