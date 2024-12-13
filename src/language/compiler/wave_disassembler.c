#include "wave_disassembler.h"

#include "common/constants.h"
#include "common/error_codes.h"
#include "common/macros.h"

#include "common/data/string/hash.h"
#include "common/data/string/string.h"

#include "language/wave_limits.h"
#include "language/wave_opcodes.h"

#include "language/compiler/wave_compiler.h"

// Functions

error_code wave_disassemble(wave_vm* vm, wave_disassembler_print_function print_function) {
    const wave_memory_allocation_function allocate_memory = vm->allocate_memory;
    const wave_memory_reallocation_function reallocate_memory = vm->reallocate_memory;
    const wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    // accessing bytecode

    register byte* bytecode_start = vm->bytecode_start;
    register byte* bytecode_end = vm->bytecode_end;
    register byte* bytecode = bytecode_start;

    #define GET_TYPE(type) (*((type*) bytecode))

    #define GET_U8()  GET_TYPE(u8)
    #define GET_U16() GET_TYPE(u16)
    #define GET_U32() GET_TYPE(u32)
    #define GET_U64() GET_TYPE(u64)

    #define GET_I8()  GET_TYPE(i8)
    #define GET_I16() GET_TYPE(i16)
    #define GET_I32() GET_TYPE(i32)
    #define GET_I64() GET_TYPE(i64)

    #define GET_F32() GET_TYPE(f32)
    #define GET_F64() GET_TYPE(f64)

    #define GET_BYTE() GET_TYPE(byte)

    #define NEXT_OFFSET(offset) do { bytecode += (offset); } while (0)
    #define NEXT_TYPE(type) NEXT_OFFSET(sizeof(type))

    #define NEXT_8()  NEXT_TYPE(u8)
    #define NEXT_16() NEXT_TYPE(u16)
    #define NEXT_32() NEXT_TYPE(u32)
    #define NEXT_64() NEXT_TYPE(u64)

    #define NEXT_BYTE() NEXT_TYPE(byte)

    // print macros

    str print_buffer = NULL;
    u32 print_buffer_size = 128;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &print_buffer, sizeof(char) * print_buffer_size);

    #define PRINT_STRING(string) RUN_ERROR_CODE_FUNCTION_TRACELESS(print_function, string, STRING_LENGTH(string))

    #define OPCODE_FORMAT_PREFIX "%{-}>5u %{ }*r %<:8s"
    #define OPCODE_FORMAT OPCODE_FORMAT_PREFIX " : "
    #define OPCODE_ARGUMENTS bytecode_offset, indentation, (str_format_data) wave_opcode_get_name(opcode)
    #define PRINT_FORMAT(format, ...)                                                                                   \
        do {                                                                                                            \
            str_format_data print_vars[] = { (str_format_data) 0, __VA_ARGS__ };                                        \
                                                                                                                        \
            u32 print_string_length = 0;                                                                                \
            str_format("%n" format "\n", print_vars, NULL, &print_string_length);                                       \
            if (print_string_length >= print_buffer_size) {                                                             \
                print_buffer_size = print_string_length + 1;                                                            \
                RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &print_buffer, sizeof(char) * print_buffer_size);   \
            }                                                                                                           \
                                                                                                                        \
            str_format("%n" format "\n", print_vars, print_buffer, &print_string_length);                               \
            RUN_ERROR_CODE_FUNCTION_TRACELESS(print_function, print_buffer, print_string_length);                       \
        } while (0)

    // print general information

    u64 bytecode_size = bytecode_end - bytecode;
    PRINT_FORMAT("bytecode size: %u64 bytes", bytecode_size);
    if (bytecode_size < WAVE_LIMIT_BYTECODE_MIN_SIZE_BYTES) {
        PRINT_FORMAT("bytecode incomplete");
        return ERROR_CODE_EXECUTION_SUCCESSFUL;
    } else if (bytecode_size > WAVE_LIMIT_BYTECODE_MAX_SIZE_BYTES) {
        PRINT_FORMAT("bytecode overflowing");
        return ERROR_CODE_EXECUTION_SUCCESSFUL;
    }

    PRINT_STRING("\n");

    // read function hash

    PRINT_FORMAT("builtin function hash: %x64", GET_TYPE(string_hash)); NEXT_TYPE(string_hash);

    PRINT_FORMAT("entrypoint branch offset: %u32 (%u32)", GET_U32() + sizeof(u16), GET_U32()); NEXT_32();
    PRINT_FORMAT("exposed function index size: %u32", GET_U32()); NEXT_32();

    PRINT_STRING("\n");

    // processing loop

    register wave_opcode opcode = OPCODE_END;

    #define CHECK_OUT_OF_BOUNDS(instruction_parameter_size)                                 \
        do {                                                                                \
            if ((instruction_parameter_size) > (bytecode_end - bytecode)) {                 \
                PRINT_FORMAT(OPCODE_FORMAT "[ incomplete instruction ]", OPCODE_ARGUMENTS); \
                goto wave_disassemble_end;                                                   \
            }                                                                               \
        } while (0)

    #define PRINT_INSTRUCTION(instruction_parameter_size, description_format, ...)          \
        do {                                                                                \
            CHECK_OUT_OF_BOUNDS(instruction_parameter_size);                                \
            PRINT_FORMAT(OPCODE_FORMAT description_format, OPCODE_ARGUMENTS, __VA_ARGS__);  \
        } while (0)

    u32 indentation = 0;

    u64 bytecode_offset = bytecode - bytecode_start;
    while (bytecode_end - bytecode > 0) {
        opcode = (wave_opcode) GET_BYTE(); NEXT_BYTE();
        switch (opcode) {
            case OPCODE_CJUMP:

            case OPCODE_CJUMP_8_IF_0:
            case OPCODE_CJUMP_8_IF_1:
            case OPCODE_CJUMP_16_IF_0:
            case OPCODE_CJUMP_16_IF_1:
            case OPCODE_CJUMP_32_IF_0:
            case OPCODE_CJUMP_32_IF_1:
            case OPCODE_CJUMP_64_IF_0:
            case OPCODE_CJUMP_64_IF_1: {
                PRINT_INSTRUCTION(sizeof(u16), "[ 16bit branch_offset = %u ]", GET_U16());
                NEXT_16();
                break;
            }

            case OPCODE_CALL_NATIVE:
            case OPCODE_CALL_NATIVE_ERR: {
                PRINT_INSTRUCTION(sizeof(i16), "[ 16bit function_index = %u ]", GET_U16());
                NEXT_16();
                break;
            }

            case OPCODE_CALL: {
                PRINT_INSTRUCTION(sizeof(u32), "[ 32bit branch_offset = %u ]", GET_U32());
                NEXT_32();
                break;
            }

            case OPCODE_ERR_TRY_START: {
                PRINT_INSTRUCTION(sizeof(u32), "[ 32bit branch_offset = %u ]", GET_U32());
                NEXT_32();
                break;
            }

            case OPCODE_PUSH_8:  { PRINT_INSTRUCTION(sizeof(u8),  "[ 8bit value = %u ]",  GET_U8());  NEXT_8();  break; }
            case OPCODE_PUSH_16: { PRINT_INSTRUCTION(sizeof(u16), "[ 16bit value = %u ]", GET_U16()); NEXT_16(); break; }
            case OPCODE_PUSH_32: { PRINT_INSTRUCTION(sizeof(u32), "[ 32bit value = %u ]", GET_U32()); NEXT_32(); break; }
            case OPCODE_PUSH_64: { PRINT_INSTRUCTION(sizeof(u64), "[ 64bit value = %u ]", GET_U64()); NEXT_64(); break; }

            case OPCODE_POP_N: {
                PRINT_INSTRUCTION(sizeof(u16), "[ 16bit amount_bytes = %u ]", GET_U16());
                NEXT_16();
                break;
            }

            case OPCODE_LOAD_8:
            case OPCODE_LOAD_16:
            case OPCODE_LOAD_32:
            case OPCODE_LOAD_64:
            case OPCODE_STORE_8:
            case OPCODE_STORE_16:
            case OPCODE_STORE_32:
            case OPCODE_STORE_64: {
                PRINT_INSTRUCTION(sizeof(u16), "[ 16bit offset = %u ]", GET_U16());
                NEXT_16();
                break;
            }

            case OPCODE_GET_GLOB_8:
            case OPCODE_GET_GLOB_16:
            case OPCODE_GET_GLOB_32:
            case OPCODE_GET_GLOB_64:
            case OPCODE_SET_GLOB_8:
            case OPCODE_SET_GLOB_16:
            case OPCODE_SET_GLOB_32:
            case OPCODE_SET_GLOB_64: {
                PRINT_INSTRUCTION(sizeof(u32), "[ 32bit offset = %u ]", GET_U32());
                NEXT_32();
                break;
            }

            case OPCODE_STR_NEW: {
                CHECK_OUT_OF_BOUNDS(sizeof(u32));
                u32 length = GET_U32(); NEXT_32();
                CHECK_OUT_OF_BOUNDS(length);
                PRINT_FORMAT(OPCODE_FORMAT "[ 32bit length = %u | str string_data = \"%s\" ]", OPCODE_ARGUMENTS, length, (str_format_data) ((str) bytecode), length);
                NEXT_OFFSET(length);
                break;
            }

            case OPCODE_STRUCT_GET_8:
            case OPCODE_STRUCT_GET_16:
            case OPCODE_STRUCT_GET_32:
            case OPCODE_STRUCT_GET_64:
            case OPCODE_STRUCT_SET_8:
            case OPCODE_STRUCT_SET_16:
            case OPCODE_STRUCT_SET_32:
            case OPCODE_STRUCT_SET_64: {
                PRINT_INSTRUCTION(sizeof(u16), "[ 16bit offset = %u ]", GET_U16());
                NEXT_16();
                break;
            }

            case OPCODE_TYPE_CONV_STATIC:
            case OPCODE_TYPE_CONV_REINTERPRET: {
                CHECK_OUT_OF_BOUNDS(sizeof(u8));

                byte parameter = GET_BYTE(); NEXT_BYTE();
                wave_type convert_from = (wave_type) ((parameter >> 4) & 0b00001111);
                wave_type convert_to = (wave_type) ((parameter >> 0) & 0b00001111);

                PRINT_FORMAT(
                    OPCODE_FORMAT
                    "[ 8bit type : (4bit type_from = %u (%s), 4bit type_to = %u (%s)) ]",
                    OPCODE_ARGUMENTS,
                    convert_from,
                    (str_format_data) wave_type_get_string(convert_from),
                    convert_to,
                    (str_format_data) wave_type_get_string(convert_to)
                );

                break;
            }

            case OPCODE_DEBUG: {
                debug_instruction_type type = (debug_instruction_type) GET_BYTE(); NEXT_BYTE();
                switch (type) {
                    case DEBUG_INSTRUCTION_TYPE_FUNCTION_START: {
                        string_hash function_name_hash = GET_U64(); NEXT_64();

                        u32 root_set_size = GET_U32(); NEXT_32();
                        NEXT_OFFSET(root_set_size);

                        u32 function_name_length = GET_U32(); NEXT_32();
                        char* function_name_start = (char*) bytecode;
                        NEXT_OFFSET(function_name_length);
                        NEXT_OFFSET(sizeof(u16) * 2);
                        u16 function_parameter_size = GET_U16(); NEXT_16();
                        u16 function_locals_stack_frame_size_size = GET_U16(); NEXT_16();

                        if (function_name_length == 0) {
                            PRINT_FORMAT("entrypoint:");
                        } else {
                            PRINT_FORMAT("function: %*s (%x64) : [ parameter = %u; stack_frame = %u ]", function_name_length, (str_format_data) function_name_start, function_name_hash, function_parameter_size, function_locals_stack_frame_size_size);
                        }

                        indentation = 0;
                        break;
                    }

                    case DEBUG_INSTRUCTION_TYPE_FUNCTION_END: {
                        if (bytecode_end - bytecode > 0) {
                            PRINT_FORMAT();
                        }

                        indentation = 0;
                        break;
                    }

                    case DEBUG_INSTRUCTION_TYPE_SCOPE_START: {
                        indentation += 4;
                        break;
                    }

                    case DEBUG_INSTRUCTION_TYPE_SCOPE_END: {
                        indentation = 0;
                        break;
                    }

                    default: {
                        break;
                    }
                }

                break;
            }

            default: {
                PRINT_FORMAT(OPCODE_FORMAT_PREFIX, OPCODE_ARGUMENTS);
                break;
            }
        }

        bytecode_offset = bytecode - bytecode_start;
    }

    wave_disassemble_end: {}

    #undef OPCODE_FORMAT
    #undef OPCODE_ARGUMENTS
    #undef PRINT_STRING
    #undef PRINT_FORMAT
    #undef CHECK_OUT_OF_BOUNDS
    #undef PRINT_INSTRUCTION

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
