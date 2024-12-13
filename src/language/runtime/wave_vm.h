#ifndef WAVE_LANGUAGE_VM
#define WAVE_LANGUAGE_VM

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "common/data/string/hash.h"

#include "language/wave_common.h"

// Defines

#define WAVE_VM_INIT_DEFAULT_PARAMETERS 32, 2048, 64 * 3, 256

// Typedefs

typedef struct {
    wave_memory_allocation_function allocate_memory;
    wave_memory_allocation_zero_function allocate_zero_memory;
    wave_memory_reallocation_function reallocate_memory;
    wave_memory_deallocation_function deallocate_memory;

    byte* bytecode_start; // pointer to the start of the compiled bytecode including the @function_hash
    byte* bytecode_end; // pointer to the end of the compiled bytecode
    byte* bytecode_current; // pointer to the start of the next instruction

    struct exposed_function_offset {
        string_hash function_name_hash;
        u32 function_offset;
    }* exposed_functions; // stores an array of the exposed functions // TODO: sort the table in the bytecode and search for the entry function via binary search

    wave_native_function* native_functions; // only used whilst initializing; stores additional information about native functions
    wave_native_function_callback* native_function_callbacks; // stack holding all registered native functions
    u32 function_stack_length; // length of the function stack
    u32 function_stack_element; // current free element in the stack (only used in the initialization phase)
    string_hash function_hash; // all native function names as a hash value to check if the current vm uses the same functions as the one used to compile the bytecode

    u32 error_branch_offset; // the offset to jump if an error occurs, 0 if it should be thrown
    error_code* error_stack_start; // pointer to the start of the error stack; the error stack tracks the accumulating error codes to either be handled by the user or the vm
    error_code* error_stack_end; // pointer to the end of the error stack
    error_code* error_stack_top; // pointer to the top of the error stack
    u32 error_stack_size;

    byte* stack_start; // pointer to the start of the stack; the stack is the main memory and stores all values in order without padding
    byte* stack_end; // pointer to the end of the stack
    byte* stack_top; // pointer to the top of the stack
    u32 stack_size;

    u32* call_stack_start; // pointer to the start of the call stack; the call stack keeps the instruction pointers from before a function was called in order to return to them later when the function is finished
    u32* call_stack_end; // pointer to the end of the call stack
    u32* call_stack_top; // pointer to the top of the call stack
    u32 call_stack_size;

    byte* globals_start; // pointer to the start of the global array; the global array can be accessed at any time and stores every type without padding for global usage inside of one running vm
    u32 globals_length; // length of the global array
    u32 globals_size;

    bool execution_finished; // whether the vm has finished execution
    number result; // the result of the bytecode execution
} wave_vm;

// Functions

error_code wave_vm_initialize(wave_vm* out_vm, wave_memory_allocation_function allocate_memory, wave_memory_allocation_zero_function allocate_zero_memory, wave_memory_reallocation_function reallocate_memory, wave_memory_deallocation_function deallocate_memory);

void wave_vm_set_stack_sizes(wave_vm* vm, u32 error_stack_size, u32 stack_size, u32 call_stack_size, u32 globals_size); // if this function is called before the source is compiled, the compiler will throw an error if any stack overflows
error_code wave_vm_register_function(wave_vm* vm, wave_native_function function);
error_code wave_vm_function_registration_done(wave_vm* vm);

error_code wave_vm_initialize_runtime(wave_vm* vm, u32 error_stack_size, u32 stack_size, u32 call_stack_size, u32 globals_size);
error_code wave_vm_begin_execution(wave_vm* vm);
error_code wave_vm_begin_function_execution(wave_vm* vm, string_hash function_name);

error_code wave_vm_destroy(wave_vm* vm);

#endif
