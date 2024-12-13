#ifndef WAVE_LANGUAGE_VM_INLINE
#define WAVE_LANGUAGE_VM_INLINE

// Includes

#include "common/constants.h"
#include "common/defines.h"
#include "common/error_codes.h"

#include "common/data/string/string.h"

#include "common/math/primitives/f32_math.h"
#include "common/math/primitives/f64_math.h"
#include "common/math/primitives/iint_math.h"
#include "common/math/primitives/uint_math.h"

#include "language/wave_common.h"
#include "language/wave_opcodes.h"

#include "language/runtime/wave_vm.h"

#endif

// compile-time dependent functions

#ifndef WAVE_VM_EXECUTE_FUNCTION_NAME
#define WAVE_VM_EXECUTE_FUNCTION_NAME wave_vm_execute /* the name of the function */
#endif

#ifndef WAVE_VM_SAFE_MODE
#define WAVE_VM_SAFE_MODE (1) /* enables more checks for stack overflow, if value is set to 1 */
#endif

#define WAVE_VM_EXECUTE_ALL (0) /* used in WAVE_VM_INSTRUCTION_EXECUTION_COUNT to execute instructions till @OPCODE_END is hit */
#ifndef WAVE_VM_INSTRUCTION_EXECUTION_COUNT
#define WAVE_VM_INSTRUCTION_EXECUTION_COUNT WAVE_VM_EXECUTE_ALL /* how many instructions should be executed per function call */
#endif

// accessing the stack

#define WAVE_VM_STACK_INLINE_DEFINE (1) /* define the macros for accessing the stack */
#define WAVE_VM_STACK_INLINE_THROW_ERRORS (1) /* use custom error handling (default is a return statement) */
#include "wave_vm_stack_inline.h" /* macros for accessing the stack; variables named "stack_start", "stack_end" & "stack" need to be defined in order for the macros to function properly */

/* wave_vm_execute
*
* Executes the bytecode by performing the following steps:
*     1. the opcode is read
*     2. the opcodes specific parameters are read (if there are any)
*     3. the instructions specific code is executed
*     4. the instruction pointer is moved to the end of the current instruction or the start of the next instruction
*
*     5. if @OPCODE_END was hit in step 1, stop the processing loop (if defined) and return the result
*
* this function comes in different versions (see defines above), that slightly alter its behaviour
*
* */
error_code WAVE_VM_EXECUTE_FUNCTION_NAME(wave_vm* vm) {
    error_code return_value = ERROR_CODE_EXECUTION_SUCCESSFUL;

    #if WAVE_VM_SAFE_MODE
    if (vm->execution_finished) {
        return ERROR_CODE_EXECUTION_SUCCESSFUL;
    }
    #endif

    // allocation functions

    const wave_memory_allocation_function allocate_memory = vm->allocate_memory;
    const wave_memory_allocation_zero_function allocate_zero_memory = vm->allocate_zero_memory;
    const wave_memory_reallocation_function reallocate_memory = vm->reallocate_memory;
    const wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    // accessing bytecode

    register byte* bytecode_start = vm->bytecode_start;
    register byte* bytecode_end = vm->bytecode_end;
    register byte* bytecode = vm->bytecode_current;

    #define GET_TYPE(type, offset) (*((type*) (bytecode + offset)))

    #define GET_U8()  GET_TYPE(u8,  0)
    #define GET_U16() GET_TYPE(u16, 0)
    #define GET_U32() GET_TYPE(u32, 0)
    #define GET_U64() GET_TYPE(u64, 0)

    #define GET_I8()  GET_TYPE(i8,  0)
    #define GET_I16() GET_TYPE(i16, 0)
    #define GET_I32() GET_TYPE(i32, 0)
    #define GET_I64() GET_TYPE(i64, 0)

    #define GET_F32() GET_TYPE(f32, 0)
    #define GET_F64() GET_TYPE(f64, 0)

    #define GET_BYTE() GET_TYPE(byte, 0)

    #define NEXT_OFFSET(offset) do { bytecode += (offset); } while (0)
    #define NEXT_TYPE(type) NEXT_OFFSET(sizeof(type))

    #define NEXT_8()  NEXT_TYPE(u8)
    #define NEXT_16() NEXT_TYPE(u16)
    #define NEXT_32() NEXT_TYPE(u32)
    #define NEXT_64() NEXT_TYPE(u64)

    #define NEXT_BYTE() NEXT_TYPE(byte)

    // error stack

    error_code* error_stack_start = vm->error_stack_start;
    error_code* error_stack_end = vm->error_stack_end;
    error_code* error_stack = vm->error_stack_top;

    register error_code temp_error_code = ERROR_CODE_EXECUTION_SUCCESSFUL;

    #define ERROR_STACK_PUSH(error)                 \
        do {                                        \
            if (error_stack < error_stack_end) {    \
                *error_stack = error;               \
                error_stack++;                      \
            }                                       \
        } while (0)

    #define THROW_ERROR(error)                                          \
        do {                                                            \
            if (vm->error_branch_offset != 0) {                         \
                ERROR_STACK_PUSH(error);                                \
                bytecode = bytecode_start + vm->error_branch_offset;    \
                goto wave_vm_execute_next_instruction;                   \
            } else {                                                    \
                temp_error_code = error;                                \
                goto wave_vm_execute_throw_error;                        \
            }                                                           \
        } while (0)

    // stack

    register byte* stack_start = vm->stack_start;
    register byte* stack_end = vm->stack_end;
    register byte* stack = vm->stack_top;

    // call stack (in order: parent_instruction_pointer, child_instruction_pointer, stack_frame)

    u32* call_stack_start = vm->call_stack_start;
    u32* call_stack_end = vm->call_stack_end;
    u32* call_stack = vm->call_stack_top;

    // globals

    byte* globals_start = vm->globals_start;
    u32 globals_length = vm->globals_length;
    byte* globals_end = globals_start + globals_length;

    // functions

    /* Function Calls, Returns & Error Handling
    *
    * FUNCTION BYTECODE STRUCTURE: [ ... | globals_root_set : (16bit offset...) | locals_root_set : (16bit offset...) | 16bit globals_root_set_size | 16bit locals_root_set_size | 16bit parameter_size | 16bit locals_stack_frame_size | ... ]
    *
    *     A Function is preceded by its root sets @globals_root_set and @locals_root_set used in error handling (see @OPCODE_ERR_THROW)
    *     and their corresponding sizes (@globals_root_set_size, @locals_root_set_size) and after that the size of the local stack frame @locals_stack_frame_size.
    *     A root set is generated by the compiler and keeps track of any variables that are stored on the heap, so that they can be properly deallocated
    *     once an exception is thrown inside a function.
    *
    * CALL STACK STRUCTURE: { ..., 32bit parent_instruction_pointer, 32bit child_instruction_pointer, 32bit stack_frame }
    *
    *     parent_instruction_pointer (32bit) - points to the next instruction that is executed once the function call finishes
    *     child_instruction_pointer (32bit)  - points to the first instruction of the function that is being called
    *     stack_frame (32bit)                - points to the start of the functions stack frame
    *
    *     Before the first instruction of the program can be executed the call stack needs to be initialized with the entrypoints
    *     function call data (@call_stack.@parent_instruction_pointer, @call_stack.@child_instruction_pointer, @call_stack.@stack_frame).
    *
    * When a function is called, the following steps are performed:
    *     0. the parameters need to be pushed to the stack before the function call is made
    *        parameters are then dealt with as local variables and also need to be tracked in the locals root set
    *
    *     1. the instruction pointer to the next instruction (@call_stack.@parent_instruction_pointer) is pushed to the call stack
    *     2. the child instruction pointer (@call_stack.@child_instruction_pointer), which points to the first instruction of the function that was called, is pushed to the call stack
    *     3. the current position in the stack (@call_stack.@stack_frame) is pushed to the call stack
    *
    *     4. the instruction pointer is moved to the beginning of the first instruction and @locals_stack_frame_size is read, then the
    *        stack pointer is incremented by the size of the local variables in the function (@locals_stack_frame_size)
    *
    *     5. the first and the following instructions are executed
    *
    *     6. once a return instruction (@OPCODE_RETURN) is hit, the parent instruction pointer (@call_stack.@parent_instruction_pointer) is popped off the call stack
    *        together with the function call data (@call_stack.@parent_instruction_pointer, @call_stack.@child_instruction_pointer, @call_stack.@stack_frame) and the instruction pointer is
    *        set to @parent_instruction_pointer
    *
    * SIDENOTE: see @OPCODE_CALL_x, @OPCODE_RETURN, @OPCODE_LOAD_x and @OPCODE_STORE_x for more information.
    *
    * When an exception is thrown inside a function, the following steps are performed:
    *     1. the start of the function stack frame (@call_stack.@stack_frame) is popped off the call stack
    *     2. the child instruction pointer (@call_stack.@child_instruction_pointer) is popped off the call stack
    *     3. the child instruction pointer (@call_stack.@parent_instruction_pointer) is popped off the call stack
    *
    *     4. the instruction pointer is moved to @call_stack.@parent_instruction_pointer and @globals_root_set_size and @locals_root_set_size are read
    *     5. the instruction pointer is moved to the start of @locals_root_set and each 16bit value stored
    *        inside @locals_root_set is queried and its corresponding value in the locals stack is deallocated if it is not null
    *     6. the instruction pointer is moved to the start of @globals_root_set_size and each 16bit value stored
    *        inside @globals_root_set is queried and its corresponding value in the globals array is deallocated if it is not null
    *
    *     7. the instruction pointer is moved back to the parent function
    *
    * */

    // processing loop

    register wave_opcode opcode = OPCODE_END;

    #if WAVE_VM_INSTRUCTION_EXECUTION_COUNT == WAVE_VM_EXECUTE_ALL
    while (true) {
    #else
    for (u32 i = 0; i < WAVE_VM_INSTRUCTION_EXECUTION_COUNT; i++) {
    #endif

        opcode = (wave_opcode) GET_BYTE();
        NEXT_BYTE();

        switch (opcode) {
            case OPCODE_END: {
                /* Instruction Description:
                *
                * Halts the execution of the bytecode immediately, returning the largest possible value that
                * can be popped off the stack (8bit, 16bit, 32bit, 64bit).
                * */

                goto wave_vm_execute_end;
            }

            case OPCODE_NOP: {
                /* Instruction Description:
                *
                * Essentially does nothing. Is used in the compilation process when aligning chunks of bytecode
                * and assembling functions. May also be used for debugging purposes.
                * Whether an error should be thrown when encountering this opcode is implementation defined. (debug mode -> breakpoint)
                * */

                #if WAVE_VM_SAFE_MODE
                return ERROR_CODE_LANGUAGE_RUNTIME_ENCOUNTERED_NOP_INSTRUCTION;
                #endif

                break;
            }

            ////////////////////////////////////////////////////////////////
            // Instruction Pointer                                        //
            ////////////////////////////////////////////////////////////////

            case OPCODE_JUMP: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @branch_offset (32bit, i32) - the offset to jump by relative to the end of this instruction
                *
                * Pops @branch_offset off the stack and jumps by @branch_offset
                * relative to the end of this instruction.
                *
                * Parameters are popped off the stack.
                * */

                i32 offset = 0; STACK_POP(offset);

                #if WAVE_VM_SAFE_MODE != 0
                if ((bytecode + offset) < bytecode_start || (bytecode + offset) > bytecode_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                }
                #endif

                NEXT_OFFSET(offset);
                break;
            }

            case OPCODE_CJUMP: {
                /* Instruction Bytecode: [ opcode | 32bit branch_offset (i32) ]
                *
                *     @branch_offset (32bit, i32) - the offset to jump by relative to the end of this instruction
                *
                * Jumps by @branch_offset relative to the end of this instruction.
                * */

                i32 offset = GET_I32(); NEXT_32();

                #if WAVE_VM_SAFE_MODE != 0
                if ((bytecode + offset) < bytecode_start || (bytecode + offset) > bytecode_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                }
                #endif

                NEXT_OFFSET(offset);
                break;
            }

            #if WAVE_VM_SAFE_MODE == 0
                #define OPCODE_IMPL_CJUMP_IF(compare_operation, type)   \
                    do {                                                \
                        i16 offset = GET_I16(); NEXT_16();              \
                        type value = 0; STACK_GET(value, 0);            \
                                                                        \
                        if (value compare_operation 0) {                \
                            STACK_POP_BYTES(sizeof(type));              \
                            NEXT_OFFSET(offset);                        \
                        }                                               \
                    } while (0)
            #else
                #define OPCODE_IMPL_CJUMP_IF(compare_operation, type)                                       \
                    do {                                                                                    \
                        i16 offset = GET_I16(); NEXT_16();                                                  \
                        type value = 0; STACK_GET(value, 0);                                                \
                                                                                                            \
                        if ((bytecode + offset) < bytecode_start || (bytecode + offset) > bytecode_end) {   \
                            THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);                \
                        }                                                                                   \
                                                                                                            \
                        if (value compare_operation 0) {                                                    \
                            STACK_POP_BYTES(sizeof(type));                                                  \
                            NEXT_OFFSET(offset);                                                            \
                        }                                                                                   \
                    } while (0)
            #endif

            /* Instruction Bytecode: [ opcode | 16bit branch_offset ]
            *
            *     @branch_offset (16bit) - the offset to jump by relative to the end of this instruction
            *
            * Stack Parameters: (bottom -> top)
            *
            *     @value (x bit) - the boolean value
            *
            * Pops @value off the stack and jumps by @branch_offset,
            * if @value is 0 (OPCODE_CJUMP_x_IF_0) or if @value
            * is not equal to 0 (OPCODE_CJUMP_x_IF_1).
            * */
            case OPCODE_CJUMP_8_IF_0:  { OPCODE_IMPL_CJUMP_IF(==, u8);  break; }
            case OPCODE_CJUMP_8_IF_1:  { OPCODE_IMPL_CJUMP_IF(!=, u8);  break; }
            case OPCODE_CJUMP_16_IF_0: { OPCODE_IMPL_CJUMP_IF(==, u16); break; }
            case OPCODE_CJUMP_16_IF_1: { OPCODE_IMPL_CJUMP_IF(!=, u16); break; }
            case OPCODE_CJUMP_32_IF_0: { OPCODE_IMPL_CJUMP_IF(==, u32); break; }
            case OPCODE_CJUMP_32_IF_1: { OPCODE_IMPL_CJUMP_IF(!=, u32); break; }
            case OPCODE_CJUMP_64_IF_0: { OPCODE_IMPL_CJUMP_IF(==, u64); break; }
            case OPCODE_CJUMP_64_IF_1: { OPCODE_IMPL_CJUMP_IF(!=, u64); break; }

            #undef OPCODE_IMPL_CJUMP_IF

            case OPCODE_TABLESWITCH: {
                /* Instruction Bytecode: [ opcode | 16bit field : (2bit value_size, 14bit length) | array jump_table : (16bit branch_offset) ]
                *
                *     @field (16bit):
                *          2bit : @value_size - the size of the top value in the stack, to be used to look up the @branch_offset from the table (8bit - 64bit)
                *         14bit : @length     - the length of the jump table
                *     @jump_table (16bit * @length):
                *         16bit : @branch_offset - the offset to jump by relative to the end of this instruction
                *
                * The value of the size @value_size is popped off the stack and the instruction pointer moved to the end of this
                * instruction and then incremented by the @branch_offset at the index of the value (from the stack) in the array @jump_table.
                * */

                u16 field = GET_U16(); NEXT_16();
                u16 length = field & 0b0011111111111111;
                if (length == 0) {
                    break;
                }

                wave_type value_type = (wave_type) (field >> (U16_BIT_COUNT - 2));
                u64 table_index = 0;
                switch (value_type) {
                    case WAVE_TYPE_U8:  { STACK_GET_U8(table_index,  0); break; }
                    case WAVE_TYPE_U16: { STACK_GET_U16(table_index, 0); break; }
                    case WAVE_TYPE_U32: { STACK_GET_U32(table_index, 0); break; }
                    case WAVE_TYPE_U64: { STACK_GET_U64(table_index, 0); break; }

                    default: {
                        return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE;
                    }
                }

                #if WAVE_VM_SAFE_MODE != 0
                if (table_index >= length) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE);
                }
                #endif

                NEXT_OFFSET(sizeof(u16) * table_index);
                u16 branch_offset = GET_U16();

                #if WAVE_VM_SAFE_MODE != 0
                if ((bytecode + branch_offset) > bytecode_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                }
                #endif

                NEXT_OFFSET(sizeof(u16) * (length - table_index) + branch_offset); // jump to the end of this instruction and jump by the offset from the jump table

                break;
            }

            case OPCODE_LOOKUPSWITCH: {
                /* Instruction Bytecode: [ opcode | 16bit field : (2bit value_size, 14bit length) | array jump_table : (value, 16bit branch_offset) ]
                *
                *     @field (16bit):
                *          2bit : @value_size - the size of the top value in the stack, to be used to look up the @branch_offset from the table (8bit - 64bit)
                *         14bit : @length     - the length of the jump table
                *     @jump_table ((@value_size bits + 16bit) * @length):
                *         x bit : @value - the value to the corresponding @branch_offset
                *         16bit : @branch_offset - the offset to jump by relative to the end of this instruction
                *
                * The value of the size @value_size is popped off the stack and then searched in @jump_table using a
                * binary search algorithm (best case complexity: O(log(n))). If the value was found the instruction pointer is
                * moved to the end of this instruction and then incremented by the found @branch_offset.
                *
                * Parameters are popped off the stack.
                * */

                u16 field = GET_U16(); NEXT_16();
                u16 length = field & 0b0011111111111111;
                if (length == 0) {
                    break;
                }

                wave_type value_type = (wave_type) (field >> (U16_BIT_COUNT - 2));
                umax value_size = 0b1 << value_type; // should be equivalent to using sizeof

                u64 value = 0;
                switch (value_type) {
                    case WAVE_TYPE_U8:  { STACK_POP_TYPE(u8,  value); break; }
                    case WAVE_TYPE_U16: { STACK_POP_TYPE(u16, value); break; }
                    case WAVE_TYPE_U32: { STACK_POP_TYPE(u32, value); break; }
                    case WAVE_TYPE_U64: { STACK_POP_TYPE(u64, value); break; }

                    default: {
                        return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE;
                    }
                }

                u16 branch_offset = 0;
                bool found_offset = false;

                u16 bound_left = 0;
                u16 bound_right = length - 1;
                while (bound_left != bound_right) { // binary search in the jump table
                    u16 middle_index = (bound_left + bound_right) / 2;
                    u64 middle_value = 0;
                    umax bytecode_offset = (value_size * middle_index) + (sizeof(u16) * (middle_index == 0 ? 0 : middle_index - 1));
                    switch (value_type) { // [ array jump_table : (value, 16bit : branch_offset) ]
                        case WAVE_TYPE_U8:  { middle_value = (u64) (*((u8*)  bytecode + bytecode_offset)); break; }
                        case WAVE_TYPE_U16: { middle_value = (u64) (*((u16*) bytecode + bytecode_offset)); break; }
                        case WAVE_TYPE_U32: { middle_value = (u64) (*((u32*) bytecode + bytecode_offset)); break; }
                        case WAVE_TYPE_U64: { middle_value = (u64) (*((u64*) bytecode + bytecode_offset)); break; }

                        default: {
                            return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE;
                        }
                    }

                    if (middle_value > value) {
                        bound_right = middle_index - 1;
                    } else if (middle_value < value) {
                        bound_left = middle_index;
                    }

                    if (middle_value == value) {
                        branch_offset = *((u16*) (bytecode + ((value_size + sizeof(u16)) * middle_index)));
                        goto wave_vm_execute_lookupswitch_end;
                    }
                }

                return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE; // if we land here, an error occurred

                wave_vm_execute_lookupswitch_end: {}

                NEXT_OFFSET((value_size + sizeof(u16)) * length); // jump to the end of this instruction

                #if WAVE_VM_SAFE_MODE != 0
                if ((bytecode + branch_offset) > bytecode_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                }
                #endif

                NEXT_OFFSET(branch_offset);
                break;
            }

            ////////////////////////////////////////////////////////////////
            // Functions                                                  //
            ////////////////////////////////////////////////////////////////

            case OPCODE_CALL_NATIVE: {
                /* Instruction Bytecode: [ opcode | 16bit function_index ]
                *
                *     @function_index (16bit) - the index of the native function to call
                *
                * Calls the native function at index @function_index in the function stack of the vm.
                * Arguments need to be pushed to the stack before calling and are popped off the stack
                * by the vm or the native function itself.
                *
                * This instruction may not run native functions that can throw error codes.
                * */

                u16 function_index = GET_U16(); NEXT_16();

                #if WAVE_VM_SAFE_MODE != 0
                if (function_index >= vm->function_stack_length) {
                    return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_NATIVE_FUNCTION_CALL;
                }
                #endif

                byte* temp_stack_top = stack;
                vm->native_function_callbacks[function_index](stack_start, stack_end, stack, &temp_stack_top);
                stack = temp_stack_top;
                break;
            }

            case OPCODE_CALL_NATIVE_ERR: {
                /* Instruction Bytecode: [ opcode | 16bit function_index ]
                *
                *     @function_index (16bit) - the index of the native function to call
                *
                * Calls the native function at index @function_index in the function stack of the vm.
                * Arguments need to be pushed to the stack before calling and are popped off the stack
                * by the vm or the native function itself.
                *
                * If the native function returns an error code, the corresponding exception is thrown
                * and the stack of the current function, including its parameters
                * is cleared back to before the function call was made.
                *
                * @OPCODE_ERR_CHECK must follow after this instruction, if the error is not caught with @OPCODE_ERR_CATCH.
                *
                * see @OPCODE_ERR_THROW and @OPCODE_ERR_CHECK for further explanation
                * */

                u16 function_index = GET_U16(); NEXT_16();

                #if WAVE_VM_SAFE_MODE != 0
                if (function_index >= vm->function_stack_length) {
                    return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_NATIVE_FUNCTION_CALL;
                }
                #endif

                byte* temp_stack_top = stack;
                error_code function_result = vm->native_function_callbacks[function_index](stack_start, stack_end, stack, &temp_stack_top);
                stack = temp_stack_top;
                if (function_result != ERROR_CODE_EXECUTION_SUCCESSFUL) {
                    ERROR_STACK_PUSH(function_result); // the error_code is only pushed to the stack, to later be handled by @OPCODE_ERR_CHECK
                }

                break;
            }

            case OPCODE_CALL: {
                /* Instruction Bytecode: [ opcode | 32bit branch_offset (i32) ]
                *
                *     @branch_offset (32bit, i32) - the offset to jump by relative to the end of this instruction
                *
                * The current stack frame and instruction pointer are pushed to the call stack and then
                * the instruction pointer is moved to @branch_offset.
                *
                * This instruction may not run functions that can throw error codes.
                * */

                if ((call_stack + 3) > call_stack_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_CALL_STACK_OVERFLOW);
                }

                u32 branch_offset = GET_U32(); NEXT_32();
                #if WAVE_VM_SAFE_MODE != 0
                if ((bytecode_start + branch_offset) > bytecode_end) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                }
                #endif

                u32 parent_instruction_pointer = bytecode - bytecode_start;
                u32 child_instruction_pointer = (bytecode + branch_offset) - bytecode_start;

                bytecode = bytecode_start + branch_offset;

                u16 parameter_size = GET_U16(); NEXT_16();
                u16 locals_stack_frame_size = GET_U16(); NEXT_16();

                *call_stack = (typeof(*call_stack)) parent_instruction_pointer; call_stack++; // parent instruction pointer
                *call_stack = (typeof(*call_stack)) child_instruction_pointer; call_stack++; // child instruction pointer (used in error handling)
                *call_stack = (typeof(*call_stack)) STACK_GET_TOP() - parameter_size + locals_stack_frame_size; call_stack++; // stack frame

                stack += locals_stack_frame_size;
                break;
            }

            case OPCODE_CALL_DYN: {
                /* Stack Parameters: (bottom -> top)
                *
                *    @function_identifier (32bit):
                *          1bit  : @native_function    - whether the function that is being called is a native function
                *          31bit : @function_call_data - the call data of the function, for native functions it is their index, otherwise the branch_offset
                *
                * If @native_function is set, the native function at the index @function_call_data in the function stack of the vm is called.
                * If @native_function is not set, the current stack frame and instruction pointer are pushed to the call stack and then the instruction pointer
                * is moved to the end of this instruction and incremented by @function_call_data.
                * In the case of a normal function, the bit that follows @native_function of @function_call_data becomes the sign bit of the branch offset.
                *
                * @function_identifier is popped off the stack before the call is made.
                *
                * This instruction may not run functions that can throw error codes.
                * */

                u32 function_identifier = 0; STACK_POP_TYPE(u32, function_identifier);

                if ((function_identifier & (0b1 << (U32_BIT_COUNT - 1))) != 0) { // native function
                    u16 function_index = function_identifier & U16_BIT_1;

                    byte* temp_stack_top = stack;
                    vm->native_function_callbacks[function_index](stack_start, stack_end, stack, &temp_stack_top);
                    stack = temp_stack_top;
                } else {
                    u32 branch_offset = (function_identifier & (U32_BIT_1 >> 1));

                    if ((call_stack + 3) > call_stack_end) {
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_CALL_STACK_OVERFLOW);
                    }

                    #if WAVE_VM_SAFE_MODE != 0
                    if ((bytecode_start + branch_offset) > bytecode_end) {
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                    }
                    #endif

                    *call_stack = (typeof(*call_stack)) (bytecode - bytecode_start); call_stack++; // parent instruction pointer
                    *call_stack = (typeof(*call_stack)) ((bytecode + branch_offset) - bytecode_start); call_stack++; // child instruction pointer (used in error handling)
                    *call_stack = (typeof(*call_stack)) (stack - stack_start); call_stack++; // stack frame (used in error handling)

                    bytecode = bytecode_start + branch_offset;
                }

                break;
            }

            case OPCODE_CALL_DYN_ERR: {
                /* Stack Parameters: (bottom -> top)
                *
                *    @function_identifier (32bit):
                *          1bit  : @native_function    - whether the function that is being called is a native function
                *          31bit : @function_call_data - the call data of the function, for native functions it is their index, otherwise the branch_offset
                *
                * If @native_function is set, the native function at the index @function_call_data in the function stack of the vm is called.
                * If @native_function is not set, the current stack frame and instruction pointer are pushed to the call stack and then the instruction pointer
                * is moved to the end of this instruction and incremented by @function_call_data.
                * In the case of a normal function, the bit that follows @native_function of @function_call_data becomes the sign bit of the branch offset.
                *
                * @function_identifier is popped off the stack before the call is made.
                *
                * If the native function returns an error code, the corresponding exception is thrown
                * and the stack of the current function, including its parameters
                * is cleared back to before the function call was made.
                * If the native function can return error codes and the error is not caught with @OPCODE_ERR_CATCH,
                * @OPCODE_ERR_CHECK must follow after this instruction (see @OPCODE_ERR_THROW and @OPCODE_ERR_CHECK for further explanation).
                *
                * This instruction may not run functions that can throw error codes.
                * */

                u32 function_identifier = 0; STACK_GET_U32(function_identifier, 0);

                if ((function_identifier & (0b1 << (U32_BIT_COUNT - 1))) != 0) { // native function
                    u16 function_index = function_identifier & U16_BIT_1;

                    byte* temp_stack_top = stack;
                    error_code function_result = vm->native_function_callbacks[function_index](stack_start, stack_end, stack, &temp_stack_top);
                    stack = temp_stack_top;
                    if (function_result != ERROR_CODE_EXECUTION_SUCCESSFUL) {
                        ERROR_STACK_PUSH(function_result); // the error_code is only pushed to the stack, to later be handled by @OPCODE_ERR_CHECK
                    }
                } else {
                    u32 temp = (function_identifier & (U32_BIT_1 >> 2));
                    i32 branch_offset = *((i32*) &temp);
                    if ((function_identifier & (0b1 << (U32_BIT_COUNT - 2))) != 0) {
                        branch_offset = -branch_offset;
                    }

                    if ((call_stack + 3) > call_stack_end) {
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_CALL_STACK_OVERFLOW);
                    }

                    #if WAVE_VM_SAFE_MODE != 0
                    if ((bytecode + branch_offset) > bytecode_end) {
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE);
                    }
                    #endif

                    *call_stack = (typeof(*call_stack)) (bytecode - bytecode_start); call_stack++; // parent instruction pointer
                    *call_stack = (typeof(*call_stack)) ((bytecode + branch_offset) - bytecode_start); call_stack++; // child instruction pointer (used in error handling)
                    *call_stack = (typeof(*call_stack)) (stack - stack_start); call_stack++; // stack frame (used in error handling)

                    NEXT_OFFSET(branch_offset);
                }

                break;
            }

            case OPCODE_RETURN: {
                /* Instruction Description:
                *
                * Returns from a function, by setting the instruction pointer to parent instruction pointer
                * (stored in the call stack) and then popping the function call data off the call stack (parent instruction pointer,
                * child instruction pointer, stack frame).
                * */

                #if WAVE_VM_SAFE_MODE != 0
                if (call_stack < call_stack_start) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_CALL_STACK_UNDERFLOW);
                }
                #endif

                call_stack -= 3; // pop parent instruction pointer, child instruction pointer and stack frame
                bytecode = bytecode_start + (umax) *call_stack; // retrieve parent instruction pointer
                break;
            }

            ////////////////////////////////////////////////////////////////
            // Error Handling                                             //
            ////////////////////////////////////////////////////////////////

            case OPCODE_ERR_THROW: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @error_code (16bit) - the error code that is being thrown
                *
                * Pops @error_code off the stack, pushes it to the error stack, throws the corresponding exception and clears the
                * stack of the current function, including its parameters back to before the function call was made.
                *
                * FUNCTION BYTECODE STRUCTURE: [ ... | globals_root_set : (16bit offset...) | locals_root_set : (16bit offset...) | 16bit globals_root_set_size | 16bit locals_root_set_size | 16bit locals_stack_frame_size | function start | ... ]
                * CALL STACK STRUCTURE: { ..., 32bit parent_instruction_pointer, 32bit child_instruction_pointer, 32bit stack_frame }
                *
                * For the stack clearing to work properly, heap objects that have been allocated and were not deallocated
                * before the error was thrown must be deallocated accordingly. To properly deallocate all heap objects without slowing down "normal"
                * function calls, each function stores an array of offsets (@locals_root_set) in front of the first instruction of the function, whose values
                * point to heap objects relative to the start of the function stack frame.
                * In order to deallocate heap objects passed to the global array, a root set (@globals_root_set) for the globals is stored in front of @locals_root_set.
                * Both the size of @globals_root_set (@globals_root_set_size) and @locals_root_set (@locals_root_set_size) is stored in front of the first instruction of the function.
                *
                * Function calls should always skip the root sets and move the instruction pointer directly to the first instruction. Everything needed for clearing the
                * stack (@call_stack.@parent_instruction_pointer, @call_stack.@child_instruction_pointer, @call_stack.@stack_frame) is pushed to the call stack by the respective CALL opcodes.
                *
                * for more information regarding functions, see the comment about functions up above
                * */

                temp_error_code = ERROR_CODE_EXECUTION_SUCCESSFUL; STACK_POP_TYPE(error_code, temp_error_code);
                ERROR_STACK_PUSH(temp_error_code); // we don't care if the error stack is full, returning an error here would not be sensible

                // this error handling code is also used elsewhere in the vm
                // before jumping to @wave_vm_execute_throw_error temp_error needs to be set to the error code that should be thrown
                wave_vm_execute_throw_error: {}

                // pop parent & child instruction pointer and stack frame off the call stack

                call_stack--; typeof(*call_stack) stack_frame = *call_stack; // the stack offset relative to the start of the stack from before the function was called including the parameters
                call_stack--; typeof(*call_stack) child_instruction_pointer = *call_stack; // points to the start of the first instruction that is going to be executed once the function is called
                call_stack--; typeof(*call_stack) parent_instruction_pointer = *call_stack; // points to the start of the instruction that follows the function call

                if (parent_instruction_pointer == 0) { // reached top scope (entrypoint function)
                    return_value = temp_error_code; // return thrown error code
                    goto wave_vm_execute_end; // end execution because we reached entrypoint scope
                }

                // move the instruction pointer to the start of the function and read @globals_root_set_size and @locals_root_set_size

                bytecode = bytecode_start + child_instruction_pointer;
                u16 globals_root_set_size = GET_TYPE(u16, -(sizeof(u16) * 2)); // see offset above
                u16 locals_root_set_size  = GET_TYPE(u16, -(sizeof(u16) * 1)); // see offset above

                // deallocate locals

                if (locals_root_set_size > 0) {
                    bytecode -= sizeof(u16) + sizeof(u16) + locals_root_set_size; // move the instruction pointer to the start of @locals_root_set_size
                    stack = stack_start + stack_frame; // move the stack pointer to the start of the local variables

                    for (u32 i = 0; i < locals_root_set_size; i++) {
                        u16 offset = ((u16 *) bytecode)[i];
                        addr address = *((addr *) (stack + offset));
                        if (address != NULL) {
                            RUN_ERROR_CODE_FUNCTION(deallocate_memory, address);
                        }
                    }
                }

                // deallocate globals

                if (globals_root_set_size > 0) {
                    bytecode -= globals_root_set_size; // move the instruction pointer to the start of @globals_root_set_size

                    for (u32 i = 0; i < globals_root_set_size; i++) {
                        u16 offset = ((u16 *) bytecode)[i];
                        addr address = *((addr *) (globals_start + offset));
                        if (address != NULL) {
                            RUN_ERROR_CODE_FUNCTION(deallocate_memory, address);
                        }
                    }
                }

                // move the instruction pointer back to the parent function

                bytecode = bytecode_start + parent_instruction_pointer;
                break;
            }

            case OPCODE_ERR_TRY_START: {
                /* Instruction Bytecode: [ opcode | 32bit branch_offset ]
                *
                *     @branch_offset (32bit) - the offset relative to the start of the bytecode, that the instruction pointer is set to
                *
                * Starts the error catching by storing @branch_offset until @OPCODE_ERR_CATCH or @OPCODE_ERR_CHECK is hit.
                * When an error occurs the instruction pointer is moved to @branch_offset and the error is handled by the user
                * The compiler then needs to either put @OPCODE_ERR_CATCH or @OPCODE_ERR_CHECK or both behind this instruction.
                *
                * The error can be read using @OPCODE_ERR_READ and compared with regular if statements.
                * */

                vm->error_branch_offset = GET_U32(); NEXT_32();
                break;
            }

            case OPCODE_ERR_CATCH: {
                /* Instruction Description:
                *
                * Catches the thrown exceptions, by popping the top error_code off the error stack
                * and exiting the do-catch statement (by setting @vm.@error_branch_offset to 0).
                * */

                if (error_stack > error_stack_start) {
                    error_stack--;
                }

                vm->error_branch_offset = 0;
                break;
            }

            case OPCODE_ERR_READ: {
                /* Instruction Description:
                *
                * Reads the 16bit error_code from the error stack and pushes it to the stack.
                * If the error stack is empty @ERROR_CODE_EXECUTION_SUCCESSFUL is pushed to the stack.
                * */

                if (error_stack == error_stack_start) {
                    STACK_PUSH_16(ERROR_CODE_EXECUTION_SUCCESSFUL);
                } else {
                    STACK_PUSH_16(*(error_stack - 1));
                }

                break;
            }

            case OPCODE_ERR_CHECK: {
                /* Instruction Description:
                *
                * Reads the 16bit error_code from the error stack, if present, throws the corresponding
                * exception and then exits the do-catch statement (by setting @vm.@error_branch_offset to 0).
                *
                * see @OPCODE_ERR_THROW for further explanation
                * */

                vm->error_branch_offset = 0;

                if (error_stack == error_stack_start) {
                    break; // the error stack is empty, no exceptions were thrown
                }

                temp_error_code = *(error_stack - 1);
                goto wave_vm_execute_throw_error; // throw the error
                break;
            }

            ////////////////////////////////////////////////////////////////
            // Stack                                                      //
            ////////////////////////////////////////////////////////////////

            case OPCODE_PUSH_8:  { STACK_PUSH_8(GET_U8());   NEXT_8();  break; }
            case OPCODE_PUSH_16: { STACK_PUSH_16(GET_U16()); NEXT_16(); break; }
            case OPCODE_PUSH_32: { STACK_PUSH_32(GET_U32()); NEXT_32(); break; }
            case OPCODE_PUSH_64: { STACK_PUSH_64(GET_U64()); NEXT_64(); break; }

            case OPCODE_POP_8:   { STACK_POP_8();  break; }
            case OPCODE_POP_16:  { STACK_POP_16(); break; }
            case OPCODE_POP_32:  { STACK_POP_32(); break; }
            case OPCODE_POP_64:  { STACK_POP_BYTES(sizeof(u32) * 2); break; } // also used when 2 32bit values have to be popped
            case OPCODE_POP_128: { STACK_POP_BYTES(sizeof(u64) * 2); break; }

            case OPCODE_POP_N: {
                /* Instruction Bytecode: [ opcode | 16bit amount_bytes ]
                *
                *     @amount_bytes (16bit)  - how many bytes to pop off the stack
                *
                * Reads @amount_bytes from the stack and then pops as many bytes as @amount_bytes.
                * @bytes is not included in this calculation.
                *
                * If the value of @amount_bytes is equal to @U16_MAX the entire stack is cleared.
                * */

                u16 amount_bytes = 0; STACK_GET_U16(amount_bytes, 0);
                if (amount_bytes == U16_MAX) {
                    STACK_POP_BYTES_UNSAFE(STACK_GET_TOP());
                } else {
                    STACK_POP_BYTES(amount_bytes);
                }

                break;
            }

            case OPCODE_POP_FREE: {
                /* Instruction Description:
                *
                * Pops an address of the stack and then deallocates its assigned
                * block of memory.
                * */

                addr address = 0;

                #if WAVE_VM_SAFE_MODE != 0
                if (STACK_GET_TOP() >= sizeof(typeof(address))) {
                #endif

                address = STACK_ACCESS(typeof(address), 0);

                #if WAVE_VM_SAFE_MODE != 0
                } else {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);
                }
                #endif

                RUN_ERROR_CODE_FUNCTION(deallocate_memory, address);
                stack -= sizeof(typeof(address));

                break;
            }

            case OPCODE_SWAP_8:  { STACK_SWAP_TYPE(u8);  break; }
            case OPCODE_SWAP_16: { STACK_SWAP_TYPE(u16); break; }
            case OPCODE_SWAP_32: { STACK_SWAP_TYPE(u32); break; }
            case OPCODE_SWAP_64: { STACK_SWAP_TYPE(u64); break; }

            //case OPCODE_DUP_8:  { STACK_DUP_TYPE(u8);  break; }
            //case OPCODE_DUP_16: { STACK_DUP_TYPE(u16); break; }
            //case OPCODE_DUP_32: { STACK_DUP_TYPE(u32); break; }
            //case OPCODE_DUP_64: { STACK_DUP_TYPE(u64); break; }

            //case OPCODE_PULL_8:  { u16 offset = GET_U16(); NEXT_16(); STACK_PULL_TYPE(u8,  offset); break; }
            //case OPCODE_PULL_16: { u16 offset = GET_U16(); NEXT_16(); STACK_PULL_TYPE(u16, offset); break; }
            //case OPCODE_PULL_32: { u16 offset = GET_U16(); NEXT_16(); STACK_PULL_TYPE(u32, offset); break; }
            //case OPCODE_PULL_64: { u16 offset = GET_U16(); NEXT_16(); STACK_PULL_TYPE(u64, offset); break; }

            ////////////////////////////////////////////////////////////////
            // Local Stack                                                //
            ////////////////////////////////////////////////////////////////

            /* Instruction Bytecode: [ opcode | 16bit offset ]
            *
            *     @offset (16bit) - the offset of the variable from the start of the local stack (function stack frame)
            *
            * Pushes a value from the local stack (function stack frame) at
            * the offset @offset to the top of the stack.
            * */
            case OPCODE_LOAD_8: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                STACK_PUSH_8(*((u8*) (stack_start + stack_frame + offset)));
                break;
            }

            case OPCODE_LOAD_16: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                STACK_PUSH_16(*((u16*) (stack_start + stack_frame + offset)));
                break;
            }

            case OPCODE_LOAD_32: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                STACK_PUSH_32(*((u32*) (stack_start + stack_frame + offset)));
                break;
            }

            case OPCODE_LOAD_64: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                STACK_PUSH_64(*((u64*) (stack_start + stack_frame + offset)));
                break;
            }

            /* Instruction Bytecode: [ opcode | 16bit offset ]
            *
            *     @offset (16bit) - the offset of the variable from the start of the local stack (function stack frame)
            *
            * Stack Parameters: (bottom -> top)
            *
            *     @value (x bit) - the value to be stored
            *
            * Pops a value off the stack and sets the variable at @offset in
            * the local stack (function stack frame) to that value.
            *
            * Parameters are not popped off the stack.
            * */
            case OPCODE_STORE_8: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                u8 value = 0; STACK_POP_TYPE(u8, value);
                *((u8*) (stack_start + stack_frame + offset)) = value;
                break;
            }

            case OPCODE_STORE_16: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                u16 value = 0; STACK_POP_TYPE(u16, value);
                *((u16*) (stack_start + stack_frame + offset)) = value;
                break;
            }

            case OPCODE_STORE_32: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                u32 value = 0; STACK_POP_TYPE(u32, value);
                *((u32*) (stack_start + stack_frame + offset)) = value;
                break;
            }

            case OPCODE_STORE_64: {
                u16 offset = GET_U16(); NEXT_16();
                typeof(*call_stack) stack_frame = *(call_stack - 1);
                u64 value = 0; STACK_POP_TYPE(u64, value);
                *((u64*) (stack_start + stack_frame + offset)) = value;
                break;
            }

            ////////////////////////////////////////////////////////////////
            // Global Stack                                               //
            ////////////////////////////////////////////////////////////////

            #define OPCODE_IMPL_GET_GLOB(type, type_size)                                       \
                do {                                                                            \
                    u32 index = 0; STACK_GET_U32(index, 0);                                     \
                    if ((index + (sizeof(type) - 1)) >= globals_length) {                       \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_GLOBALS_INDEX_OUT_OF_BOUNDS);   \
                    }                                                                           \
                                                                                                \
                    byte* temp = globals_start + index + (sizeof(type) - 1);                    \
                    CONCAT(STACK_PUSH_, type_size)(*((type*) temp));                            \
                } while (0)

            case OPCODE_GET_GLOB_8:  { OPCODE_IMPL_GET_GLOB(u8,  8);  break; }
            case OPCODE_GET_GLOB_16: { OPCODE_IMPL_GET_GLOB(u16, 16); break; }
            case OPCODE_GET_GLOB_32: { OPCODE_IMPL_GET_GLOB(u32, 32); break; }
            case OPCODE_GET_GLOB_64: { OPCODE_IMPL_GET_GLOB(u64, 64); break; }

            #define OPCODE_IMPL_SET_GLOB(type, type_name)                                       \
                do {                                                                            \
                    u32 index = 0; STACK_GET_U32(index, 0);                                     \
                    if ((index + (sizeof(type) - 1)) >= globals_length) {                       \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_GLOBALS_INDEX_OUT_OF_BOUNDS);   \
                    }                                                                           \
                                                                                                \
                    type value = 0; CONCAT(STACK_GET_, type_name)(value, sizeof(u32));          \
                    byte* temp = globals_start + index + (sizeof(type) - 1);                    \
                    *((type*) temp) = value;                                                    \
                } while (0)

            case OPCODE_SET_GLOB_8:  { OPCODE_IMPL_SET_GLOB(u8,  U8);  break; }
            case OPCODE_SET_GLOB_16: { OPCODE_IMPL_SET_GLOB(u16, U16); break; }
            case OPCODE_SET_GLOB_32: { OPCODE_IMPL_SET_GLOB(u32, U32); break; }
            case OPCODE_SET_GLOB_64: { OPCODE_IMPL_SET_GLOB(u64, U64); break; }

            #undef OPCODE_IMPL_GET_GLOB
            #undef OPCODE_IMPL_SET_GLOB

            ////////////////////////////////////////////////////////////////
            // Bit / Bitwise Operations                                   //
            ////////////////////////////////////////////////////////////////

            // Bit Shift

            case OPCODE_SHIFT_L_8:  { STACK_OPERATION_BINARY_ASSIGN(u8,  <<=); break; }
            case OPCODE_SHIFT_L_16: { STACK_OPERATION_BINARY_ASSIGN(u16, <<=); break; }
            case OPCODE_SHIFT_L_32: { STACK_OPERATION_BINARY_ASSIGN(u32, <<=); break; }
            case OPCODE_SHIFT_L_64: { STACK_OPERATION_BINARY_ASSIGN(u64, <<=); break; }

            case OPCODE_SHIFT_R_8:  { STACK_OPERATION_BINARY_ASSIGN(u8,  >>=); break; }
            case OPCODE_SHIFT_R_16: { STACK_OPERATION_BINARY_ASSIGN(u16, >>=); break; }
            case OPCODE_SHIFT_R_32: { STACK_OPERATION_BINARY_ASSIGN(u32, >>=); break; }
            case OPCODE_SHIFT_R_64: { STACK_OPERATION_BINARY_ASSIGN(u64, >>=); break; }

            // Bitwise And

            case OPCODE_BAND_8:  { STACK_OPERATION_BINARY_ASSIGN(u8,  &=); break; }
            case OPCODE_BAND_16: { STACK_OPERATION_BINARY_ASSIGN(u16, &=); break; }
            case OPCODE_BAND_32: { STACK_OPERATION_BINARY_ASSIGN(u32, &=); break; }
            case OPCODE_BAND_64: { STACK_OPERATION_BINARY_ASSIGN(u64, &=); break; }

            // Bitwise Or

            case OPCODE_BOR_8:  { STACK_OPERATION_BINARY_ASSIGN(u8,  |=); break; }
            case OPCODE_BOR_16: { STACK_OPERATION_BINARY_ASSIGN(u16, |=); break; }
            case OPCODE_BOR_32: { STACK_OPERATION_BINARY_ASSIGN(u32, |=); break; }
            case OPCODE_BOR_64: { STACK_OPERATION_BINARY_ASSIGN(u64, |=); break; }

            // Bitwise Or

            case OPCODE_XOR_8:  { STACK_OPERATION_BINARY_ASSIGN(u8,  ^=); break; }
            case OPCODE_XOR_16: { STACK_OPERATION_BINARY_ASSIGN(u16, ^=); break; }
            case OPCODE_XOR_32: { STACK_OPERATION_BINARY_ASSIGN(u32, ^=); break; }
            case OPCODE_XOR_64: { STACK_OPERATION_BINARY_ASSIGN(u64, ^=); break; }

            // Bitwise Not

            case OPCODE_BNOT_8:  { STACK_OPERATION_UNARAY(u8,  ~); break; }
            case OPCODE_BNOT_16: { STACK_OPERATION_UNARAY(u16, ~); break; }
            case OPCODE_BNOT_32: { STACK_OPERATION_UNARAY(u32, ~); break; }
            case OPCODE_BNOT_64: { STACK_OPERATION_UNARAY(u64, ~); break; }

            // Comparing

            case OPCODE_NOT_8:  { STACK_OPERATION_UNARAY(u8,  !); break; }
            case OPCODE_NOT_16: { STACK_OPERATION_UNARAY(u16, !); break; }
            case OPCODE_NOT_32: { STACK_OPERATION_UNARAY(u32, !); break; }
            case OPCODE_NOT_64: { STACK_OPERATION_UNARAY(u64, !); break; }

            case OPCODE_EQU_8:  { STACK_OPERATION_BINARY(u8,  ==); break; }
            case OPCODE_EQU_16: { STACK_OPERATION_BINARY(u16, ==); break; }
            case OPCODE_EQU_32: { STACK_OPERATION_BINARY(u32, ==); break; }
            case OPCODE_EQU_64: { STACK_OPERATION_BINARY(u64, ==); break; }

            case OPCODE_NEQ_8:  { STACK_OPERATION_BINARY(u8,  !=); break; }
            case OPCODE_NEQ_16: { STACK_OPERATION_BINARY(u16, !=); break; }
            case OPCODE_NEQ_32: { STACK_OPERATION_BINARY(u32, !=); break; }
            case OPCODE_NEQ_64: { STACK_OPERATION_BINARY(u64, !=); break; }

            case OPCODE_AND_8:  { STACK_OPERATION_BINARY(u8,  &&); break; }
            case OPCODE_AND_16: { STACK_OPERATION_BINARY(u16, &&); break; }
            case OPCODE_AND_32: { STACK_OPERATION_BINARY(u32, &&); break; }
            case OPCODE_AND_64: { STACK_OPERATION_BINARY(u64, &&); break; }

            case OPCODE_OR_8:  { STACK_OPERATION_BINARY(u8,  ||); break; }
            case OPCODE_OR_16: { STACK_OPERATION_BINARY(u16, ||); break; }
            case OPCODE_OR_32: { STACK_OPERATION_BINARY(u32, ||); break; }
            case OPCODE_OR_64: { STACK_OPERATION_BINARY(u64, ||); break; }

            ////////////////////////////////////////////////////////////////
            // Integer Math Functions                                     //
            ////////////////////////////////////////////////////////////////

            #define OPCODE_IMPL_U_MATH_INSTRUCTIONS(type, type_name)                                                \
                case OPCODE_##type_name##_ADD: { STACK_OPERATION_BINARY_ASSIGN(type, +=); break; }                  \
                case OPCODE_##type_name##_SUB: { STACK_OPERATION_BINARY_ASSIGN(type, -=); break; }                  \
                case OPCODE_##type_name##_MUL: { STACK_OPERATION_BINARY_ASSIGN(type, *=); break; }                  \
                case OPCODE_##type_name##_DIV: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, /=); break; }       \
                case OPCODE_##type_name##_MOD: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, %=); break; }       \
                case OPCODE_##type_name##_POW: { STACK_OPERATION_BINARY_FUNC(type, CONCAT(type, _pow)); break; }    \
                                                                                                                    \
                case OPCODE_##type_name##_INC: { STACK_OPERATION_UNARAY_POSTFIX(type, += 1); break; }               \
                case OPCODE_##type_name##_DEC: { STACK_OPERATION_UNARAY_POSTFIX(type, -= 1); break; }               \
                                                                                                                    \
                case OPCODE_##type_name##_GT: { STACK_OPERATION_BINARY(type, >);  break; }                          \
                case OPCODE_##type_name##_GE: { STACK_OPERATION_BINARY(type, >=); break; }                          \
                case OPCODE_##type_name##_LT: { STACK_OPERATION_BINARY(type, <);  break; }                          \
                case OPCODE_##type_name##_LE: { STACK_OPERATION_BINARY(type, <=); break; }

            #define OPCODE_IMPL_I_MATH_INSTRUCTIONS(type, type_name)                                                                                                    \
                case OPCODE_##type_name##_ADD: { STACK_OPERATION_BINARY_ASSIGN(type, +=); break; }                                                                      \
                case OPCODE_##type_name##_SUB: { STACK_OPERATION_BINARY_ASSIGN(type, -=); break; }                                                                      \
                case OPCODE_##type_name##_MUL: { STACK_OPERATION_BINARY_ASSIGN(type, *=); break; }                                                                      \
                case OPCODE_##type_name##_DIV: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, /=); break; }                                                           \
                case OPCODE_##type_name##_MOD: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, %=); break; }                                                           \
                case OPCODE_##type_name##_POW: { STACK_OPERATION_BINARY_FUNC(type, CONCAT(type, _pow)); break; }                                                        \
                                                                                                                                                                        \
                case OPCODE_##type_name##_INC: { STACK_OPERATION_UNARAY_POSTFIX(type, += 1); break; }                                                                   \
                case OPCODE_##type_name##_DEC: { STACK_OPERATION_UNARAY_POSTFIX(type, -= 1); break; }                                                                   \
                                                                                                                                                                        \
                case OPCODE_##type_name##_NEG: { STACK_OPERATION_UNARAY(type, -); break; }                                                                              \
                case OPCODE_##type_name##_ABS: { type value = 0; CONCAT(STACK_GET_, type_name)(value, 0); if (value < 0) { STACK_OPERATION_UNARAY(type, -); } break; }  \
                                                                                                                                                                        \
                case OPCODE_##type_name##_LT: { STACK_OPERATION_BINARY(type, <);  break; }                                                                              \
                case OPCODE_##type_name##_LE: { STACK_OPERATION_BINARY(type, <=); break; }                                                                              \
                case OPCODE_##type_name##_GT: { STACK_OPERATION_BINARY(type, >);  break; }                                                                              \
                case OPCODE_##type_name##_GE: { STACK_OPERATION_BINARY(type, >=); break; }

            OPCODE_IMPL_U_MATH_INSTRUCTIONS(u8,  U8)
            OPCODE_IMPL_U_MATH_INSTRUCTIONS(u16, U16)
            OPCODE_IMPL_U_MATH_INSTRUCTIONS(u32, U32)
            OPCODE_IMPL_U_MATH_INSTRUCTIONS(u64, U64)

            OPCODE_IMPL_I_MATH_INSTRUCTIONS(i8,  I8)
            OPCODE_IMPL_I_MATH_INSTRUCTIONS(i16, I16)
            OPCODE_IMPL_I_MATH_INSTRUCTIONS(i32, I32)
            OPCODE_IMPL_I_MATH_INSTRUCTIONS(i64, I64)

            #undef OPCODE_IMPL_U_MATH_INSTRUCTIONS
            #undef OPCODE_IMPL_I_MATH_INSTRUCTIONS

            ////////////////////////////////////////////////////////////////
            // Floating Math Functions                                    //
            ////////////////////////////////////////////////////////////////

            // F32 Math

            case OPCODE_F32_ADD: { STACK_OPERATION_BINARY_ASSIGN(f32, +=); break; }
            case OPCODE_F32_SUB: { STACK_OPERATION_BINARY_ASSIGN(f32, -=); break; }
            case OPCODE_F32_MUL: { STACK_OPERATION_BINARY_ASSIGN(f32, *=); break; }
            case OPCODE_F32_DIV: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(f32, /=); break; }
            case OPCODE_F32_MOD: { STACK_OPERATION_BINARY_FUNC_ZERO_CHECK(f32, f32_mod); break; }
            case OPCODE_F32_POW: { STACK_OPERATION_BINARY_FUNC(f32, f32_pow); break; }

            case OPCODE_F32_EQU: { STACK_OPERATION_BINARY(f32, ==); break; }
            case OPCODE_F32_NEQ: { STACK_OPERATION_BINARY(f32, !=); break; }

            case OPCODE_F32_NEG: { STACK_OPERATION_UNARAY(f32, -); break; }
            case OPCODE_F32_ABS: { f32 value = 0.0F; STACK_GET_F32(value, 0); if (value < 0.0F) { STACK_OPERATION_UNARAY(f32, -); } break; }

            case OPCODE_F32_LT: { STACK_OPERATION_BINARY(f32, <);  break; }
            case OPCODE_F32_LE: { STACK_OPERATION_BINARY(f32, <=); break; }
            case OPCODE_F32_GT: { STACK_OPERATION_BINARY(f32, >);  break; }
            case OPCODE_F32_GE: { STACK_OPERATION_BINARY(f32, >=); break; }

            // F64 Math

            case OPCODE_F64_ADD: { STACK_OPERATION_BINARY_ASSIGN(f64, +=); break; }
            case OPCODE_F64_SUB: { STACK_OPERATION_BINARY_ASSIGN(f64, -=); break; }
            case OPCODE_F64_MUL: { STACK_OPERATION_BINARY_ASSIGN(f64, *=); break; }
            case OPCODE_F64_DIV: { STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(f32, /=); break;  }
            case OPCODE_F64_MOD: { STACK_OPERATION_BINARY_FUNC_ZERO_CHECK(f64, f64_mod); break; }
            case OPCODE_F64_POW: { STACK_OPERATION_BINARY_FUNC(f64, f64_pow); break; }

            case OPCODE_F64_EQU: { STACK_OPERATION_BINARY(f64, ==); break; }
            case OPCODE_F64_NEQ: { STACK_OPERATION_BINARY(f64, !=); break; }

            case OPCODE_F64_NEG: { STACK_OPERATION_UNARAY(f64, -); break; }
            case OPCODE_F64_ABS: { f64 value = 0.0; STACK_GET_F64(value, 0); if (value < 0.0) { STACK_OPERATION_UNARAY(f64, -); } break; }

            case OPCODE_F64_LT: { STACK_OPERATION_BINARY(f64, <);  break; }
            case OPCODE_F64_LE: { STACK_OPERATION_BINARY(f64, <=); break; }
            case OPCODE_F64_GT: { STACK_OPERATION_BINARY(f64, >);  break; }
            case OPCODE_F64_GE: { STACK_OPERATION_BINARY(f64, >=); break; }


            // copies the memory from the bytecode beginning at @pointer to @pointer_end in word sizes (64/32bit)
            #if PROGRAM_FEATURE_32BIT_MODE == 0
            #define MEMORY_COPY_WORD_WISE(pointer_end, pointer)                                                                                 \
                do {                                                                                                                            \
                    while ((pointer_end - pointer) >= sizeof(u64)) {                                                                            \
                        *((u64*) pointer) = GET_U64(); NEXT_64();                                                                               \
                        pointer += sizeof(u64);                                                                                                 \
                    }                                                                                                                           \
                                                                                                                                                \
                    switch (((umax) (pointer_end - pointer))) {                                                                                 \
                        case (sizeof(u32) + sizeof(u16) + sizeof(u8)): { WORD_PUSH(u32, U32); WORD_PUSH(u16, U16); WORD_PUSH(u8, U8); break; }  \
                        case (sizeof(u32) + sizeof(u16)):              { WORD_PUSH(u32, U32); WORD_PUSH(u16, U16); break; }                     \
                        case (sizeof(u32) + sizeof(u8)):               { WORD_PUSH(u32, U32); WORD_PUSH(u8, U8); break; }                       \
                        case (sizeof(u32)):                            { WORD_PUSH(u32, U32); break; }                                          \
                                                                                                                                                \
                        case (sizeof(u16) + sizeof(u8)):               { WORD_PUSH(u16, U16); WORD_PUSH(u8, U8); break; }                       \
                        case (sizeof(u16)):                            { WORD_PUSH(u16, U16); break; }                                          \
                        case (sizeof(u8)):                             { WORD_PUSH(u8, U8); break; }                                            \
                                                                                                                                                \
                        case (0): { break; }                                                                                                    \
                                                                                                                                                \
                        default: {                                                                                                              \
                            return ERROR_CODE_LANGUAGE_RUNTIME_COPY_OPERATION_INVALID_SIZE;                                                     \
                        }                                                                                                                       \
                    }                                                                                                                           \
                } while (0)
            #else
            #define MEMORY_COPY_WORD_WISE(pointer_end, pointer)                                                                                 \
                do {                                                                                                                            \
                    while ((pointer_end - pointer) >= sizeof(u32)) {                                                                            \
                        *((u32*) pointer) = GET_U32(); NEXT_32();                                                                               \
                        pointer += sizeof(u32);                                                                                                 \
                    }                                                                                                                           \
                                                                                                                                                \
                    switch (((umax) (pointer_end - pointer))) {                                                                                 \
                        case (sizeof(u16) + sizeof(u8)):               { WORD_PUSH(u16, U16); WORD_PUSH(u8, U8); break; }                       \
                        case (sizeof(u16)):                            { WORD_PUSH(u16, U16); break; }                                          \
                        case (sizeof(u8)):                             { WORD_PUSH(u8, U8); break; }                                            \
                                                                                                                                                \
                        case (0): { break; }                                                                                                    \
                                                                                                                                                \
                        default: {                                                                                                              \
                            return ERROR_CODE_LANGUAGE_RUNTIME_COPY_OPERATION_INVALID_SIZE;                                                     \
                        }                                                                                                                       \
                    }                                                                                                                           \
                } while (0)
            #endif

            ////////////////////////////////////////////////////////////////
            // Complex Data Structure Opcodes                             //
            ////////////////////////////////////////////////////////////////

            // String

            case OPCODE_STR_NEW: {
                /* Instruction Bytecode: [ opcode | 32bit length | str string_data ]
                *
                *     @length (32bit) - the length of @string_data in bytes
                *     @string_data (n bits) - the uncompressed string data (not null-terminated)
                *
                * Allocates a new string with the length @length and the data from @string_data.
                * The string is preceded by its length (32bit) in the assigned memory block.
                * The address of the string is then pushed to the stack.
                * @string_data should be as long as @length, @length can also be 0, in this case the
                * string is empty, which means that only its length field is allocated.
                *
                * STRING STRUCTURE: [ 32bit length | str string_data (not null-terminated) ]
                *
                * Strings need to be popped off the stack using @POP_FREE.
                * */

                u32 length = GET_U32(); NEXT_32();

                str string_start = NULL;
                str string_end = NULL;
                str string = NULL;
                RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &string_start, sizeof(u32) + sizeof(char) * length);

                *((u32*) string_start) = length;

                if (length == 0) {
                    goto wave_vm_execute_str_new_end;
                }

                string = string_start + sizeof(u32); // move to the start of the string
                string_end = string + length;

                // copy the string word by word to potentially save time

                #define WORD_PUSH(type, type_name) *((type*) string) = GET_##type_name(); string += sizeof(type); NEXT_TYPE(type)
                MEMORY_COPY_WORD_WISE(string_end, string);
                #undef WORD_PUSH

                wave_vm_execute_str_new_end: {}

                STACK_PUSH_ADDR(string_start);
                break;
            }

            case OPCODE_STR_CONCAT: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string1 (addr) - the string to which is being appended
                *     @string2 (addr) - the string that is being appended
                *
                * Reads the top 2 strings (string1 and string2) from the stack and appends
                * the data of @string2 to the end of @string1, reallocating @string1, if necessary.
                * Appending null (string2) to string1 appends the string "NULL" to the end of @string1.
                *
                * @string1 is removed from the stack and then pushed to the top of the stack.
                * */

                // TODO: add an instruction argument that represents how many strings are stored on the top of the stack and how many should be concatenated

                str string1 = NULL; STACK_GET(string1, sizeof(addr));
                if (string1 == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                str string2 = NULL; STACK_GET(string2, 0);
                u32 length1 = *((u32*) string1);

                if (string2 == NULL) {
                    u32 length2 = 4;
                    str resized_string = string1;
                    RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &resized_string, sizeof(u32) + sizeof(char) * (length1 + length2)); // resize string1

                    STACK_POP_2BACK(addr); // pop string1 from the stack, because its concatenated address is pushed to the top of the stack

                    *((u32*) resized_string) = length1 + length2; // store new length

                    str temp = (resized_string + sizeof(u32) + (sizeof(char) * length1));
                    temp[0] = 'n';
                    temp[1] = 'u';
                    temp[2] = 'l';
                    temp[3] = 'l';

                    STACK_PUSH_ADDR(resized_string);
                    break;
                }

                u32 length2 = *((u32*) string2);

                str resized_string = string1;
                RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &resized_string, sizeof(u32) + sizeof(char) * (length1 + length2)); // resize string1

                STACK_POP_2BACK(addr); // pop string1 from the stack, because its concatenated address is pushed to the top of the stack

                *((u32*) resized_string) = length1 + length2; // store new length

                if (length1 + length2 == 0) {
                    goto wave_vm_execute_str_concat_end;
                }

                string1 = resized_string + sizeof(u32) + length1;
                string2 += sizeof(u32); // move to the start of the string

                str string_end = resized_string + sizeof(u32) + length1 + length2;

                // copy the string word by word to potentially save time

                #if PROGRAM_FEATURE_32BIT_MODE == 0
                while ((string_end - string1) >= sizeof(u64)) {
                    *((u64*) string1) = *((u64*) string2);
                    string1 += sizeof(u64);
                    string2 += sizeof(u64);
                }
                #else
                while ((string_end - string1) >= sizeof(u32)) {
                    *((u32*) string1) = *((u32*) string2);
                    string1 += sizeof(u32);
                    string2 += sizeof(u32);
                }
                #endif

                // copy the remaining bytes

                #define STR_PUSH(type) *((type*) string1) = *((type*) string2); string1 += sizeof(type); string2 += sizeof(type)

                switch (((umax) (string_end - string1))) {
                    #if PROGRAM_FEATURE_32BIT_MODE == 0
                    case (sizeof(u32) + sizeof(u16) + sizeof(u8)): { STR_PUSH(u32); STR_PUSH(u16); STR_PUSH(u8); break; }
                    case (sizeof(u32) + sizeof(u16)):              { STR_PUSH(u32); STR_PUSH(u16); break; }
                    case (sizeof(u32) + sizeof(u8)):               { STR_PUSH(u32); STR_PUSH(u8); break; }
                    case (sizeof(u32)):                            { STR_PUSH(u32); break; }
                    #endif
                    case (sizeof(u16) + sizeof(u8)):               { STR_PUSH(u16); STR_PUSH(u8); break; }
                    case (sizeof(u16)):                            { STR_PUSH(u16); break; }
                    case (sizeof(u8)):                             { STR_PUSH(u8); break; }

                    case (0): { break; }

                    default: {
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_COPY_OPERATION_INVALID_SIZE);
                    }
                }

                #undef STR_PUSH

                wave_vm_execute_str_concat_end: {}

                STACK_PUSH_ADDR(resized_string);
                break;
            }

            case OPCODE_STR_DUP: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string (addr) - the string to be duplicated
                *
                * Reads the top string @string from the stack, creates a copy
                * of it and pushes it onto the stack.
                *
                * Parameters are not popped off the stack.
                * */

                str string = NULL; STACK_GET(string, 0);
                u32 length = 0;
                if (string == NULL) {
                    length = 0; // allow the duplication of null strings
                } else {
                    length = *((u32*) string); string += sizeof(u32); // retrieve length of string and jump to the start of the string data
                }

                str new_string = NULL;
                RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &new_string, sizeof(u32) + (sizeof(char) * length));

                *((u32*) new_string) = length;

                if (length == 0) {
                    goto wave_vm_execute_str_dup_end;
                }

                str copy = new_string + sizeof(u32);
                str copy_end = new_string + length;

                // copy the string word by word to potentially save time

                #if PROGRAM_FEATURE_32BIT_MODE == 0
                while ((copy_end - copy) >= sizeof(u64)) {
                    *((u64*) copy) = *((u64*) string);
                    string += sizeof(u64);
                    copy += sizeof(u64);
                }
                #else
                while ((copy_end - copy) >= sizeof(u32)) {
                    *((u32*) copy) = *((u32*) string);
                    string += sizeof(u32);
                    copy += sizeof(u32);
                }
                #endif

                // copy the remaining bytes

                #define STR_PUSH(type) *((type*) copy) = *((type*) string); string += sizeof(type); copy += sizeof(type)

                switch (((umax) (copy_end - copy))) {
                    #if PROGRAM_FEATURE_32BIT_MODE == 0
                    case (sizeof(u32) + sizeof(u16) + sizeof(u8)): { STR_PUSH(u32); STR_PUSH(u16); STR_PUSH(u8); break; }
                    case (sizeof(u32) + sizeof(u16)):              { STR_PUSH(u32); STR_PUSH(u16); break; }
                    case (sizeof(u32) + sizeof(u8)):               { STR_PUSH(u32); STR_PUSH(u8); break; }
                    case (sizeof(u32)):                            { STR_PUSH(u32); break; }
                    #endif
                    case (sizeof(u16) + sizeof(u8)):               { STR_PUSH(u16); STR_PUSH(u8); break; }
                    case (sizeof(u16)):                            { STR_PUSH(u16); break; }
                    case (sizeof(u8)):                             { STR_PUSH(u8); break; }

                    case (0): { break; }

                    default: {
                        return ERROR_CODE_LANGUAGE_RUNTIME_COPY_OPERATION_INVALID_SIZE;
                    }
                }

                #undef STR_PUSH

                wave_vm_execute_str_dup_end: {}

                STACK_PUSH_ADDR(new_string);
                break;
            }

            case OPCODE_STR_EQU: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string1 (addr) - the first string to be compared
                *     @string2 (addr) - the second string to be compared
                *
                * Reads the top 2 strings (string1 and string2) from the stack and compares them.
                * If the 2 strings are identical, a boolean (8bit) with the value true (1) is pushed
                * to the stack, otherwise false (0) is being pushed.
                *
                * Parameters are not popped off the stack.
                * */

                str string1 = NULL; STACK_GET(string1, sizeof(addr));
                str string2 = NULL; STACK_GET(string2, 0);
                if ((string1 == NULL || string2 == NULL) && string1 != string2) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 length1 = *((u32*) string1); string1 += sizeof(u32); // retrieve length of string and jump to the start of the string data
                u32 length2 = *((u32*) string2); string2 += sizeof(u32); // retrieve length of string and jump to the start of the string data

                STACK_PUSH_8((u8) (str_is_equals_quick(string1, string2, length1, length2) ? 1 : 0));
                break;
            }

            case OPCODE_STR_GET: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string (addr) - the string to be read
                *     @index (32bit) - the index
                *
                * Reads the parameters from the stack and pushes the character (8bit) at
                * the index @index in the string @string to the stack.
                *
                * Parameters are not popped off the stack.
                * */

                str string = NULL; STACK_GET(string, sizeof(u32));
                if (string == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 length = *((u32*) string); string += sizeof(u32); // retrieve length of string and jump to the start of the string data
                u32 index = 0; STACK_GET_U32(index, 0); // get the desired index from the stack

                if (index >= length) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);
                }

                STACK_PUSH_8(((u8*) string)[index]); // push char
                break;
            }

            case OPCODE_STR_SET: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string (addr) - the string to be modified
                *     @character (8bit) - the character value
                *     @index (32bit) - the index
                *
                * Reads the parameters from the stack and sets the character at the index @index
                * in the string @string to the value of @character.
                *
                * Parameters are not popped off the stack.
                * */

                str string = NULL; STACK_GET(string, sizeof(u8) + sizeof(u32));
                if (string == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 length = *((u32*) string); string += sizeof(u32); // retrieve length of string and jump to the start of the string data
                u8 value = 0; STACK_GET_U8(value, sizeof(u32)); // get the @character value
                u32 index = 0; STACK_GET_U32(index, 0); // get the desired index from the stack
                if (index >= length) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);
                }

                string[index] = *((char*) &value);
                break;
            }

            case OPCODE_STR_LEN: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @string (str) - the string to be read
                *
                * Reads the top string @string from the stack and pushes its length (u32) to the stack.
                *
                * Parameters are not popped off the stack.
                * */

                str string = NULL; STACK_GET(string, 0);
                if (string == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 length = *((u32*) string);
                STACK_PUSH_32(length);
                break;
            }

            // Array

            case OPCODE_ARR_NEW: {
                /* Instruction Bytecode: [ opcode | 32bit field : (1bit has_data, 2bit value_size, 29bit length) | array data : (value...) ]
                *
                *     @field (32bit):
                *          1bit : @has_data   - whether the array is created empty (every value is set to 0)
                *          2bit : @value_size - the size of the top values stored in the array (8bit - 64bit)
                *         29bit : @length     - the length of the array
                *     @data ((@type bits) * @length):
                *         x bit : @value - the value
                *
                * Allocates a new array with the length @length. If @has_data is set to true
                * the array is set to @data, while respecting the size of each value (@value_size).
                * The array is preceded by its value_size (2bit) and its length (30bit) in the assigned memory block.
                * The address of the newly created array is then pushed to the stack.
                * @data should be as long as @length and in bytes as big as @length * @value_size
                *
                * ARRAY STRUCTURE: [ 32bit field : (2bit value_size, 30bit length) | array data : (value...) ]
                *
                * Arrays need to be popped off the stack using @POP_FREE.
                * */

                u32 field = GET_U32(); NEXT_32();
                bool has_data = field & (0b1 << (U32_BIT_COUNT - 1));
                u32 length = field & (U32_BIT_1 >> 3);
                wave_type value_type = (wave_type) ((field >> (U32_BIT_COUNT - 3)) & 0b011);
                umax value_size = 0b1 << value_type; // should be equivalent to using sizeof

                str array_start = NULL;
                str array_end = NULL;
                str array = NULL;

                if (!has_data) {
                    RUN_ERROR_CODE_FUNCTION(allocate_zero_memory, (void**) &array_start, sizeof(u32) + value_size * length);
                } else {
                    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &array_start, sizeof(u32) + value_size * length);
                }

                *((u32*) array_start) = (value_type << (U32_BIT_COUNT - 2)) & (length & (~((u32) 0b0) >> 2));
                if (length == 0 || !has_data) {
                    goto wave_vm_execute_arr_new_end;
                }

                array = array_start + sizeof(u32); // move to the start of the array
                array_end = array + length;

                // copy the string word by word to potentially save time

                #define WORD_PUSH(type, type_name) *((type*) array) = GET_##type_name(); array += sizeof(type); NEXT_TYPE(type)
                MEMORY_COPY_WORD_WISE(array_end, array);
                #undef WORD_PUSH

                wave_vm_execute_arr_new_end: {}

                STACK_PUSH_ADDR(array_start);
                break;
            }

            case OPCODE_ARR_GET: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @array (addr) - the array to be read
                *     @index (32bit) - the index
                *
                * Reads the parameters from the stack and pushes the value (of the size
                * of the corresponding arrays value size @array.@value_size) at the index @index in the array @array to the stack.
                *
                * Parameters are not popped off the stack.
                * */

                str array = NULL; STACK_GET(array, sizeof(u32));
                if (array == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 field = *((u32*) array); array += sizeof(u32);
                u32 length = field & (U32_BIT_1 >> 3);

                u32 index = 0; STACK_GET_U32(index, 0); // get the desired index from the stack
                if (index >= length) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);
                }

                wave_type value_type = (wave_type) ((field >> (U32_BIT_COUNT - 2)) & 0b11);
                switch (value_type) {
                    case WAVE_TYPE_U8:  { STACK_PUSH_8( ((u8*)  array)[index]); break; }
                    case WAVE_TYPE_U16: { STACK_PUSH_16(((u16*) array)[index]); break; }
                    case WAVE_TYPE_U32: { STACK_PUSH_32(((u32*) array)[index]); break; }
                    case WAVE_TYPE_U64: { STACK_PUSH_64(((u64*) array)[index]); break; }

                    default: {
                        return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE;
                    }
                }

                break;
            }

            case OPCODE_ARR_SET: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @array (arr) - the array to be modified
                *     @value (x bit) - the value
                *     @index (32bit) - the index
                *
                * Reads the parameters from the stack and sets value (of the size
                * of the corresponding arrays value size @array.@value_size) at the index @index
                * in the array to the value @value.
                *
                * Parameters are not popped off the stack.
                * */

                str array = NULL; STACK_GET(array, sizeof(u8) + sizeof(u32));
                if (array == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 field = *((u32*) array); array += sizeof(u32);
                u32 length = field & (U32_BIT_1 >> 3);

                u32 index = 0; STACK_GET_U32(index, 0); // get the desired index from the stack
                if (index >= length) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);
                }

                wave_type value_type = (wave_type) ((field >> (U32_BIT_COUNT - 2)) & 0b11);
                switch (value_type) {
                    case WAVE_TYPE_U8:  { u8 value  = 0; STACK_GET_U8(value, sizeof(u32));  ((u8*)  array)[index] = value; break; }
                    case WAVE_TYPE_U16: { u16 value = 0; STACK_GET_U16(value, sizeof(u32)); ((u16*) array)[index] = value; break; }
                    case WAVE_TYPE_U32: { u32 value = 0; STACK_GET_U32(value, sizeof(u32)); ((u32*) array)[index] = value; break; }
                    case WAVE_TYPE_U64: { u64 value = 0; STACK_GET_U64(value, sizeof(u32)); ((u64*) array)[index] = value; break; }

                    default: {
                        return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE;
                    }
                }

                break;
            }

            case OPCODE_ARR_LEN: {
                /* Stack Parameters: (bottom -> top)
                *
                *     @array (arr) - the array to be read
                *
                * Reads the top array @array from the stack and pushes its length (u32) to the stack.
                *
                * Parameters are not popped off the stack.
                * */

                str array = NULL; STACK_GET(array, 0);
                if (array == NULL) {
                    THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);
                }

                u32 field = *((u32*) array);
                u32 length = field & (U32_BIT_1 >> 3);
                STACK_PUSH_32(length);
                break;
            }

            // Struct

            case OPCODE_STRUCT_NEW: {
                /* Instruction Bytecode: [ opcode | 16bit field : (1bit has_data, 15bit size) | array data : (value...) ]
                *
                *     @field (16bit):
                *          1bit : @has_data - whether the struct is created empty (every value is set to 0)
                *         15bit : @size     - the size of the struct
                *     @data (@size bits):
                *         x bit : @value - the value
                *
                * Allocates a new struct with the size @size. If @has_data is set to true
                * the struct is set to @data.
                * The struct is preceded by its size (16bit) in the assigned memory block.
                * The address of the newly created struct is then pushed to the stack.
                * @data should be as big as @size.
                *
                * STRUCT STRUCTURE: [ 16bit size | array data : (value...) ]
                *
                * Structs need to be popped off the stack using @POP_FREE.
                * */

                u16 field = GET_U16(); NEXT_16();
                bool has_data = field & (0b1 << (U32_BIT_COUNT - 1));
                u32 size = field & 0b0111111111111111;

                str struct_start = NULL;
                str struct_end = NULL;
                str struct_current = NULL;

                if (!has_data) {
                    RUN_ERROR_CODE_FUNCTION(allocate_zero_memory, (void**) &struct_start, sizeof(u16) + size);
                } else {
                    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &struct_start, sizeof(u32) + size);
                }

                *((u16*) struct_start) = size;
                if (size == 0 || !has_data) {
                    goto wave_vm_execute_struct_new_end;
                }

                struct_current = struct_start + sizeof(u16); // move to the start of the struct
                struct_end = struct_current + size;

                // copy the string word by word to potentially save time

                #define WORD_PUSH(type, type_name) *((type*) struct_current) = GET_##type_name(); struct_current += sizeof(type); NEXT_TYPE(type)
                MEMORY_COPY_WORD_WISE(struct_end, struct_current);
                #undef WORD_PUSH

                wave_vm_execute_struct_new_end: {}

                STACK_PUSH_ADDR(struct_start);
                break;
            }

            #undef MEMORY_COPY_WORD_WISE
            #undef WORD_PUSH

            #if WAVE_VM_SAFE_MODE == 0
            #define OPCODE_IMPL_STRUCT_GET(type)                                            \
                do {                                                                        \
                    u16 offset = GET_U16();                                                 \
                    addr struct_variable = NULL; STACK_GET(struct_variable, 0);             \
                    if (struct_variable == NULL) {                                          \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);    \
                    }                                                                       \
                                                                                            \
                    type value = *((type*) (struct_variable + sizeof(u16) + offset));       \
                    STACK_PUSH_TYPE(type, value);                                           \
                } while (0)
            #else
            #define OPCODE_IMPL_STRUCT_GET(type)                                            \
                do {                                                                        \
                    u16 offset = GET_U16();                                                 \
                    addr struct_variable = NULL; STACK_GET(struct_variable, 0);             \
                    if (struct_variable == NULL) {                                          \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);    \
                    }                                                                       \
                                                                                            \
                    u32 length = (u32) *((u16*) struct_variable);                           \
                    if (offset > length) {                                                  \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);       \
                    }                                                                       \
                                                                                            \
                    type value = *((type*) (struct_variable + sizeof(u16) + offset));       \
                    STACK_PUSH_TYPE(type, value);                                           \
                } while (0)
            #endif

            /* Instruction Bytecode: [ opcode | 16bit offset ]
            *
            *     @offset (16bit) - the offset of the variable from the start of the struct
            *
            * Stack Parameters: (bottom -> top)
            *
            *     @struct (addr) - the struct to be read
            *
            * Reads the top struct @struct from the stack and pushes the variable
            * stored at the byte offset @offset in @strict to the stack.
            *
            * Parameters are not popped off the stack.
            * */
            case OPCODE_STRUCT_GET_8:  { OPCODE_IMPL_STRUCT_GET(u8);  break; }
            case OPCODE_STRUCT_GET_16: { OPCODE_IMPL_STRUCT_GET(u16); break; }
            case OPCODE_STRUCT_GET_32: { OPCODE_IMPL_STRUCT_GET(u32); break; }
            case OPCODE_STRUCT_GET_64: { OPCODE_IMPL_STRUCT_GET(u64); break; }

            #undef OPCODE_IMPL_STRUCT_GET

            #if WAVE_VM_SAFE_MODE == 0
            #define OPCODE_IMPL_STRUCT_SET(type)                                            \
                do {                                                                        \
                    u16 offset = GET_U16();                                                 \
                    addr struct_variable = NULL; STACK_GET(struct_variable, sizeof(type));  \
                    if (struct_variable == NULL) {                                          \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);    \
                    }                                                                       \
                                                                                            \
                    type value = 0; STACK_GET(value, 0);                                    \
                    *((type*) (struct_variable + sizeof(u16) + offset)) = value;            \
                } while (0)
            #else
            #define OPCODE_IMPL_STRUCT_SET(type)                                            \
                do {                                                                        \
                    u16 offset = GET_U16();                                                 \
                    addr struct_variable = NULL; STACK_GET(struct_variable, sizeof(type));  \
                    if (struct_variable == NULL) {                                          \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION);    \
                    }                                                                       \
                                                                                            \
                    u32 length = (u32) *((u16*) struct_variable);                           \
                    if (offset > length) {                                                  \
                        THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS);       \
                    }                                                                       \
                                                                                            \
                    type value = 0; STACK_GET(value, 0);                                    \
                    *((type*) (struct_variable + sizeof(u16) + offset)) = value;            \
                } while (0)
            #endif

            /* Instruction Bytecode: [ opcode | 16bit offset ]
            *
            *     @offset (16bit) - the offset of the variable from the start of the struct
            *
            * Stack Parameters: (bottom -> top)
            *
            *     @struct (addr) - the struct to be read
            *     @value (x bit) - the struct to be read
            *
            * Reads the top struct @struct from the stack and sets the variable
            * stored at the byte offset @offset in @strict to the value of @value.
            *
            * Parameters are not popped off the stack.
            * */
            case OPCODE_STRUCT_SET_8:  { OPCODE_IMPL_STRUCT_SET(u8);  break; }
            case OPCODE_STRUCT_SET_16: { OPCODE_IMPL_STRUCT_SET(u16); break; }
            case OPCODE_STRUCT_SET_32: { OPCODE_IMPL_STRUCT_SET(u32); break; }
            case OPCODE_STRUCT_SET_64: { OPCODE_IMPL_STRUCT_SET(u64); break; }

            #undef OPCODE_IMPL_STRUCT_SET

            ////////////////////////////////////////////////////////////////
            // Other Opcodes                                              //
            ////////////////////////////////////////////////////////////////

            // Type Conversion

            #define TYPE_CONVERSION_INVALID_CASE() default: { return ERROR_CODE_LANGUAGE_RUNTIME_TYPE_CONVERSION_INVALID_ARGUMENTS; }

            #define CONVERT_SWITCH(from_type, from)         \
                do {                                        \
                    switch (convert_to) {                   \
                        CONVERT(from_type, from, u8,  U8)   \
                        CONVERT(from_type, from, u16, U16)  \
                        CONVERT(from_type, from, u32, U32)  \
                        CONVERT(from_type, from, u64, U64)  \
                                                            \
                        CONVERT(from_type, from, i8,  I8)   \
                        CONVERT(from_type, from, i16, I16)  \
                        CONVERT(from_type, from, i32, I32)  \
                        CONVERT(from_type, from, i64, I64)  \
                                                            \
                        CONVERT(from_type, from, f32, F32)  \
                        CONVERT(from_type, from, f64, F64)  \
                                                            \
                        TYPE_CONVERSION_INVALID_CASE()      \
                    }                                       \
                } while (0)

            case OPCODE_TYPE_CONV_STATIC: {
                /* Instruction Bytecode: [ opcode | 8bit type : (4bit type_from, 4bit type_to) ]
                *
                *     @type (8bit):
                *         4bit : @type_from - the type of the parameter that is being converted
                *         4bit : @type_to   - the type that the parameter is being converted to
                *
                * Stack Parameters: (bottom -> top)
                *
                *     @value (8 - 64bit) - the value that is being converted
                *
                * Pops the value @value with the size of the corresponding type of @type_from off the stack
                * and converts it to the type of @type_to, which is then pushed back onto the stack.
                * */

                byte parameter = GET_BYTE(); NEXT_BYTE();
                wave_type convert_from = (wave_type) ((parameter >> 4) & 0b00001111);
                wave_type convert_to = (wave_type) ((parameter >> 0) & 0b00001111);

                #if WAVE_VM_SAFE_MODE == 0
                #define CONVERT(from_type, from_name, to_type, to_name)                                     \
                    case CONCAT(WAVE_TYPE_, to_name): {                                                      \
                        STACK_ACCESS_TYPE(from_type, to_type, 0) = (to_type) STACK_ACCESS(from_type, 0);    \
                        stack += sizeof(to_type) - sizeof(from_type);                                       \
                        break;                                                                              \
                    }
                #else
                #define CONVERT(from_type, from_name, to_type, to_name)                                                             \
                    case CONCAT(WAVE_TYPE_, to_name): {                                                                              \
                        if (STACK_GET_TOP() >= sizeof(from_type) && (stack_end - stack) >= (sizeof(from_type) + sizeof(to_type))) { \
                            STACK_ACCESS_TYPE(from_type, to_type, 0) = (to_type) STACK_ACCESS(from_type, 0);                        \
                            stack += sizeof(to_type) - sizeof(from_type);                                                           \
                        } else {                                                                                                    \
                            THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_STACK_OVERFLOW);                                                \
                        }                                                                                                           \
                                                                                                                                    \
                        break;                                                                                                      \
                    }
                #endif

                // switch statement covering all different types

                switch (convert_from) {
                    case WAVE_TYPE_U8:  { CONVERT_SWITCH(u8,  U8);  break; }
                    case WAVE_TYPE_U16: { CONVERT_SWITCH(u16, U16); break; }
                    case WAVE_TYPE_U32: { CONVERT_SWITCH(u32, U32); break; }
                    case WAVE_TYPE_U64: { CONVERT_SWITCH(u64, U64); break; }

                    case WAVE_TYPE_I8:  { CONVERT_SWITCH(i8,  I8);  break; }
                    case WAVE_TYPE_I16: { CONVERT_SWITCH(i16, I16); break; }
                    case WAVE_TYPE_I32: { CONVERT_SWITCH(i32, I32); break; }
                    case WAVE_TYPE_I64: { CONVERT_SWITCH(i64, I64); break; }

                    case WAVE_TYPE_F32: { CONVERT_SWITCH(f32, F32); break; }
                    case WAVE_TYPE_F64: { CONVERT_SWITCH(f64, F64); break; }

                    TYPE_CONVERSION_INVALID_CASE()
                }

                #undef CONVERT

                break;
            }

            case OPCODE_TYPE_CONV_REINTERPRET: {
                /* Instruction Bytecode: [ opcode | 8bit type : (4bit type_from, 4bit type_to) ]
                *
                *     @type (8bit):
                *         4bit : @type_from - the type of the parameter that is being converted
                *         4bit : @type_to   - the type that the parameter is being converted to
                *
                * Stack Parameters: (bottom -> top)
                *
                *     @value (8 - 64bit) - the value that is being converted
                *
                * Pops the value @value with the size of the corresponding type of @type_from off the stack
                * and resizes it to the size of the type of @type_to, which is then pushed back onto the stack.
                * No bits are changed in the process, only the size the variable occupies is altered (increased or reduced).
                * */

                byte parameter = GET_BYTE(); NEXT_BYTE();
                wave_type convert_from = (wave_type) ((parameter >> 4) & 0b00001111);
                wave_type convert_to = (wave_type) ((parameter >> 0) & 0b00001111);

                #if WAVE_VM_SAFE_MODE == 0
                #define CONVERT(from_type, from_name, to_type, to_name)                                         \
                    case CONCAT(WAVE_TYPE_, to_name): {                                                          \
                        STACK_ACCESS_TYPE(from_type, to_type, 0) = *((to_type*) &STACK_ACCESS(from_type, 0));   \
                        stack += sizeof(to_type) - sizeof(from_type);                                           \
                        break;                                                                                  \
                    }
                #else
                #define CONVERT(from_type, from_name, to_type, to_name)                                                             \
                    case CONCAT(WAVE_TYPE_, to_name): {                                                                              \
                        if (STACK_GET_TOP() >= sizeof(from_type) && (stack_end - stack) >= (sizeof(from_type) + sizeof(to_type))) { \
                            STACK_ACCESS_TYPE(from_type, to_type, 0) = *((to_type*) &STACK_ACCESS(from_type, 0));                   \
                            stack += sizeof(to_type) - sizeof(from_type);                                                           \
                        } else {                                                                                                    \
                            THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_STACK_OVERFLOW);                                                \
                        }                                                                                                           \
                                                                                                                                    \
                        break;                                                                                                      \
                    }
                #endif

                // switch statement covering all different types

                switch (convert_from) {
                    case WAVE_TYPE_U8:  { CONVERT_SWITCH(u8,  U8);  break; }
                    case WAVE_TYPE_U16: { CONVERT_SWITCH(u16, U16); break; }
                    case WAVE_TYPE_U32: { CONVERT_SWITCH(u32, U32); break; }
                    case WAVE_TYPE_U64: { CONVERT_SWITCH(u64, U64); break; }

                    case WAVE_TYPE_I8:  { CONVERT_SWITCH(i8,  I8);  break; }
                    case WAVE_TYPE_I16: { CONVERT_SWITCH(i16, I16); break; }
                    case WAVE_TYPE_I32: { CONVERT_SWITCH(i32, I32); break; }
                    case WAVE_TYPE_I64: { CONVERT_SWITCH(i64, I64); break; }

                    case WAVE_TYPE_F32: { CONVERT_SWITCH(f32, F32); break; }
                    case WAVE_TYPE_F64: { CONVERT_SWITCH(f64, F64); break; }

                    TYPE_CONVERSION_INVALID_CASE()
                }

                #undef CONVERT

                break;
            }

            #undef CONVERT_SWITCH

            default: { // this case should never hit, especially if 256 opcodes are defined, and indicates that an instruction was not executed properly or the compiler version differs from this version
                return ERROR_CODE_LANGUAGE_RUNTIME_INVALID_OPCODE;
            }
        }

        wave_vm_execute_next_instruction: {} // used in error handling
    }

    vm->error_stack_top = error_stack;
    vm->stack_top = stack;
    vm->call_stack_top = call_stack;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;

    // end of execution

    wave_vm_execute_end: {}

    // return result according to the size of the stack (if it is not empty)

    number result = (number) { .number_type = NUMBER_TYPE_U64, .number_value = (union_number) { .value_u64 = 0 } };
    if (return_value == ERROR_CODE_EXECUTION_SUCCESSFUL) {
        u32 current = STACK_GET_TOP();
        if (current == sizeof(byte)) {
            STACK_GET_BYTE(return_value, 0);
        } else if (current >= sizeof(error_code)) {
            STACK_GET_U16(return_value, 0);
        }
    }

    result.number_value.value_u16 = (u16) return_value;
    result.number_type = NUMBER_TYPE_U16;

    vm->execution_finished = true;
    vm->result = result;

    // macros

    #undef NEXT_BYTE
    #undef NEXT_OFFSET
    #undef NEXT_TYPE
    #undef NEXT_8
    #undef NEXT_16
    #undef NEXT_32
    #undef NEXT_64

    #undef GET_TYPE

    #undef GET_U8
    #undef GET_U16
    #undef GET_U32
    #undef GET_U64

    #undef GET_I8
    #undef GET_I16
    #undef GET_I32
    #undef GET_I64

    #undef GET_F32
    #undef GET_F64

    #undef GET_BYTE

    #undef ERROR_STACK_PUSH
    #undef THROW_ERROR

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#undef WAVE_VM_EXECUTE_FUNCTION_NAME
#undef WAVE_VM_SAFE_MODE
#undef WAVE_VM_EXECUTE_ALL
#undef WAVE_VM_INSTRUCTION_EXECUTION_COUNT

#define WAVE_VM_STACK_INLINE_DEFINE (0)
#include "wave_vm_stack_inline.h" /* un-define the stack macros to prevent warnings or errors */
