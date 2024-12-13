#if WAVE_VM_STACK_INLINE_DEFINE != 0

// throwing errors

#if WAVE_VM_STACK_INLINE_THROW_ERRORS == 0
#define THROW_ERROR(error) do { return error; } while (0)
#endif

// accessing the stack

#define STACK_TOP (stack)
#define STACK_END (stack_end)

#define STACK_GET_TOP() (stack - stack_start)
#define STACK_GET_END() (stack_end - stack_start)

#define STACK_ACCESS(type, offset) *((type*) (stack - sizeof(type) - (offset)))
#define STACK_ACCESS_TYPE(old_type, new_type, offset) *((new_type*) (stack - sizeof(old_type) - (offset)))
#define STACK_ACCESS_TOP(type) *((type*) stack)

// push

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_PUSH_TYPE(type, value)        \
        do {                                    \
            STACK_ACCESS_TOP(type) = (value);   \
            STACK_TOP += sizeof(type);          \
        } while (0)
#else
    #define STACK_PUSH_TYPE(type, value)                                    \
        do {                                                                \
            if (STACK_TOP + sizeof(type) <= STACK_END) {                    \
                STACK_ACCESS_TOP(type) = (value);                           \
                STACK_TOP += sizeof(type);                                  \
            } else {                                                        \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_STACK_OVERFLOW);    \
            }                                                               \
        } while (0)
#endif

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_PUSH_2BACK(type, value)       \
        do {                                    \
            STACK_TOP -= sizeof(type);          \
            STACK_ACCESS_TOP(type) = (value);   \
        } while (0)
#else
    #define STACK_PUSH_2BACK(type, value)                                   \
        do {                                                                \
            if (STACK_GET_CURENT() >= sizeof(type)) {                       \
                STACK_TOP -= sizeof(type);                                  \
                STACK_ACCESS_TOP(type) = (value);                           \
            } else {                                                        \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_STACK_OVERFLOW);    \
            }                                                               \
        } while (0)
#endif

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_SET_TYPE(type, value)         \
        do {                                    \
            STACK_ACCESS(type, 0) = (value);    \
        } while (0)
#else
    #define STACK_SET_TYPE(type, value)                                     \
        do {                                                                \
            if (STACK_GET_CURENT() >= sizeof(type)) {                       \
                STACK_ACCESS(type, 0) = (value);                            \
            } else {                                                        \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_STACK_OVERFLOW);    \
            }                                                               \
        } while (0)
#endif

#define STACK_PUSH_8(value)  STACK_PUSH_TYPE(u8, value)
#define STACK_PUSH_16(value) STACK_PUSH_TYPE(u16, value)
#define STACK_PUSH_32(value) STACK_PUSH_TYPE(u32, value)
#define STACK_PUSH_64(value) STACK_PUSH_TYPE(u64, value)

#define STACK_PUSH_ADDR(value) STACK_PUSH_TYPE(void*, value)

// get

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_GET_TYPE(variable, type, offset)  \
        do {                                        \
            variable = STACK_ACCESS(type, offset);  \
        } while (0)
#else
    #define STACK_GET_TYPE(variable, type, offset)                                  \
        do {                                                                        \
            if (STACK_GET_TOP() >= sizeof(type) && STACK_GET_TOP() >= (offset)) {   \
                variable = STACK_ACCESS(type, offset);                              \
            } else {                                                                \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);      \
            }                                                                       \
        } while (0)
#endif
#define STACK_GET(variable, offset) STACK_GET_TYPE(variable, typeof(variable), offset)

#define STACK_GET_U8(variable, offset)  STACK_GET_TYPE(variable, u8,  offset)
#define STACK_GET_U16(variable, offset) STACK_GET_TYPE(variable, u16, offset)
#define STACK_GET_U32(variable, offset) STACK_GET_TYPE(variable, u32, offset)
#define STACK_GET_U64(variable, offset) STACK_GET_TYPE(variable, u64, offset)

#define STACK_GET_I8(variable, offset)  STACK_GET_TYPE(variable, i8,  offset)
#define STACK_GET_I16(variable, offset) STACK_GET_TYPE(variable, i16, offset)
#define STACK_GET_I32(variable, offset) STACK_GET_TYPE(variable, i32, offset)
#define STACK_GET_I64(variable, offset) STACK_GET_TYPE(variable, i64, offset)

#define STACK_GET_F32(variable, offset) STACK_GET_TYPE(variable, f32, offset)
#define STACK_GET_F64(variable, offset) STACK_GET_TYPE(variable, f64, offset)

#define STACK_GET_BYTE(variable, offset) STACK_GET_TYPE(variable, u8, offset)

// pop

#define STACK_POP_BYTES_UNSAFE(amount_bytes)    \
    do {                                        \
        STACK_TOP -= (amount_bytes);            \
    } while (0)

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_POP_BYTES(amount_bytes) STACK_POP_BYTES_UNSAFE(amount_bytes)
#else
    #define STACK_POP_BYTES(amount_bytes)                                       \
        do {                                                                    \
            if (STACK_GET_TOP() >= (amount_bytes)) {                            \
                STACK_TOP -= (amount_bytes);                                    \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)
#endif

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_POP_TYPE(type, variable)      \
        do {                                    \
            variable = STACK_ACCESS(type, 0);   \
            STACK_TOP -= sizeof(type);          \
        } while (0)
#else
    #define STACK_POP_TYPE(type, variable)                                      \
        do {                                                                    \
            if (STACK_GET_TOP() >= sizeof(type)) {                              \
                variable = STACK_ACCESS(type, 0);                               \
                STACK_TOP -= sizeof(type);                                      \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)
#endif

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_POP_2BACK(type)                                       \
        do {                                                            \
            STACK_ACCESS(type, sizeof(type)) = STACK_ACCESS(type, 0);   \
            STACK_TOP -= sizeof(type);                                  \
        } while (0)
#else
    #define STACK_POP_2BACK(type)                                               \
        do {                                                                    \
            if (STACK_GET_TOP() >= (sizeof(type) * 2)) {                        \
                STACK_ACCESS(type, sizeof(type)) = STACK_ACCESS(type, 0);       \
                STACK_TOP -= sizeof(type);                                      \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)
#endif

#define STACK_POP(variable) STACK_POP_TYPE(typeof(variable), variable)

#define STACK_POP_8()  STACK_POP_BYTES(sizeof(u8))
#define STACK_POP_16() STACK_POP_BYTES(sizeof(u16))
#define STACK_POP_32() STACK_POP_BYTES(sizeof(u32))
#define STACK_POP_64() STACK_POP_BYTES(sizeof(u64))

// swap

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_SWAP_TYPE(type)                                       \
        do {                                                            \
            type temp = STACK_ACCESS(type, 0);                          \
            STACK_ACCESS(type, 0) = STACK_ACCESS(type, sizeof(type));   \
            STACK_ACCESS(type, sizeof(type)) = temp;                    \
        } while (0)
#else
    #define STACK_SWAP_TYPE(type)                                               \
        do {                                                                    \
            if (STACK_GET_TOP() >= (sizeof(type) * 2)) {                        \
                type temp = STACK_ACCESS(type, 0);                              \
                STACK_ACCESS(type, 0) = STACK_ACCESS(type, sizeof(type));       \
                STACK_ACCESS(type, sizeof(type)) = temp;                        \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)
#endif

// dup (duplicate the value at the top of the stack)

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_DUP_TYPE(type)                            \
        do {                                                \
            STACK_ACCESS_TOP(type) = STACK_ACCESS(type, 0); \
            STACK_TOP += sizeof(type);                      \
        } while (0)
#else
    #define STACK_DUP_TYPE(type)                                                                \
        do {                                                                                    \
            if (STACK_GET_TOP() >= sizeof(type) && (STACK_TOP + sizeof(type)) <= STACK_END) {   \
                STACK_ACCESS_TOP(type) = STACK_ACCESS(type, 0);                                 \
                STACK_TOP += sizeof(type);                                                      \
            } else {                                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);                  \
            }                                                                                   \
        } while (0)
#endif

// pull (push a value at offset to the top of the stack)

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_PULL_TYPE(type, offset)                           \
        do {                                                        \
            STACK_ACCESS_TOP(type) = STACK_ACCESS(type, offset);    \
            STACK_TOP += sizeof(type);                              \
        } while (0)
#else
    #define STACK_PULL_TYPE(type, offset)                                                                   \
        do {                                                                                                \
            if (STACK_GET_TOP() >= (sizeof(type) + (offset)) && (STACK_TOP + sizeof(type)) <= STACK_END) {  \
                STACK_ACCESS_TOP(type) = STACK_ACCESS(type, offset);                                        \
                STACK_TOP += sizeof(type);                                                                  \
            } else {                                                                                        \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);                              \
            }                                                                                               \
        } while (0)
#endif

// stack modifying operations

// ("single" means only the topmost value with the size of the corresponding type is modified;
//  "double" modifies the 2 topmost values, both the size of their type)

#if WAVE_VM_SAFE_MODE == 0
    #define STACK_OPERATION_BINARY_ASSIGN(type, operation)                      \
        do {                                                                    \
            STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);   \
            STACK_TOP -= sizeof(type);                                          \
        } while (0)

    #define STACK_OPERATION_UNARAY(type, operation)                     \
        do {                                                            \
            STACK_ACCESS(type, 0) = operation (STACK_ACCESS(type, 0));  \
        } while (0)

    #define STACK_OPERATION_UNARAY_POSTFIX(type, operation) \
        do {                                                \
            STACK_ACCESS(type, 0) operation;                \
        } while (0)

    #define STACK_OPERATION_BINARY(type, operation)                                                                 \
        do {                                                                                                        \
            STACK_ACCESS(type, sizeof(type)) = STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);    \
            STACK_TOP -= sizeof(u32);                                                                               \
        } while (0)

    #define STACK_OPERATION_BINARY_FUNC(type, function_name)                                                            \
        do {                                                                                                            \
            STACK_ACCESS(type, sizeof(type)) = function_name(STACK_ACCESS(type, sizeof(type)), STACK_ACCESS(type, 0));  \
            STACK_TOP -= sizeof(u32);                                                                                   \
        } while (0)

    #define STACK_OPERATION_BINARY_FUNC_ZERO_CHECK(type, function_name)                                                     \
        do {                                                                                                                \
            if (STACK_ACCESS(type, 0) == 0) {                                                                               \
                STACK_ACCESS_TYPE(type, type, sizeof(type)) = 0;                                                            \
            } else {                                                                                                        \
                STACK_ACCESS(type, sizeof(type)) = function_name(STACK_ACCESS(type, sizeof(type)), STACK_ACCESS(type, 0));  \
            }                                                                                                               \
                                                                                                                            \
            STACK_TOP -= sizeof(type);                                                                                      \
        } while (0)

    #define STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, operation)               \
        do {                                                                        \
            if (STACK_ACCESS(type, 0) == 0) {                                       \
                STACK_ACCESS_TYPE(type, type, sizeof(type)) = 0;                    \
            } else {                                                                \
                STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);   \
            }                                                                       \
                                                                                    \
            STACK_TOP -= sizeof(type);                                              \
        } while (0)
#else
    #define STACK_OPERATION_BINARY_ASSIGN(type, operation)                          \
        do {                                                                        \
            if (STACK_GET_TOP() >= (umax) (sizeof(type) * 2)) {                     \
                STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);   \
                STACK_TOP -= sizeof(type);                                          \
            } else {                                                                \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);      \
            }                                                                       \
        } while (0)

    #define STACK_OPERATION_UNARAY(type, operation)                             \
        do {                                                                    \
            if (STACK_GET_TOP() >= (umax) sizeof(type)) {                       \
                STACK_ACCESS(type, 0) = operation STACK_ACCESS(type, 0);        \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)

    #define STACK_OPERATION_UNARAY_POSTFIX(type, operation)                     \
        do {                                                                    \
            if (STACK_GET_TOP() >= (umax) sizeof(type)) {                       \
                STACK_ACCESS(type, 0) operation;                                \
            } else {                                                            \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);  \
            }                                                                   \
        } while (0)

    #define STACK_OPERATION_BINARY(type, operation)                                                                     \
        do {                                                                                                            \
            if (STACK_GET_TOP() >= (umax) (sizeof(type) * 2)) {                                                         \
                STACK_ACCESS(type, sizeof(type)) = STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);    \
                STACK_TOP -= sizeof(u32);                                                                               \
            } else {                                                                                                    \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);                                          \
            }                                                                                                           \
        } while (0)

    #define STACK_OPERATION_BINARY_FUNC(type, function_name)                                                                \
        do {                                                                                                                \
            if (STACK_GET_TOP() >= (umax) (sizeof(type) * 2)) {                                                             \
                STACK_ACCESS(type, sizeof(type)) = function_name(STACK_ACCESS(type, sizeof(type)), STACK_ACCESS(type, 0));  \
                STACK_TOP -= sizeof(u32);                                                                                   \
            } else {                                                                                                        \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);                                              \
            }                                                                                                               \
        } while (0)

    #define STACK_OPERATION_BINARY_FUNC_ZERO_CHECK(type, function_name)                                                     \
        do {                                                                                                                \
            if (STACK_GET_TOP() >= (umax) (sizeof(type) * 2)) {                                                             \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);                                              \
            }                                                                                                               \
                                                                                                                            \
            if (STACK_ACCESS(type, 0) == 0) {                                                                               \
                STACK_ACCESS_TYPE(type, type, sizeof(type)) = 0;                                                            \
            } else {                                                                                                        \
                STACK_ACCESS(type, sizeof(type)) = function_name(STACK_ACCESS(type, sizeof(type)), STACK_ACCESS(type, 0));  \
            }                                                                                                               \
                                                                                                                            \
            STACK_TOP -= sizeof(type);                                                                                      \
        } while (0)

    #define STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK(type, operation)               \
        do {                                                                        \
            if (STACK_GET_TOP() >= (umax) (sizeof(type) * 2)) {                     \
                THROW_ERROR(ERROR_CODE_LANGUAGE_RUNTIME_OPERATION_LEFT_STACK);      \
            }                                                                       \
                                                                                    \
            if (STACK_ACCESS(type, 0) == 0) {                                       \
                STACK_ACCESS_TYPE(type, type, sizeof(type)) = 0;                    \
            } else {                                                                \
                STACK_ACCESS(type, sizeof(type)) operation STACK_ACCESS(type, 0);   \
            }                                                                       \
                                                                                    \
            STACK_TOP -= sizeof(type);                                              \
        } while (0)
#endif

#elif WAVE_VM_STACK_INLINE_DEFINE == 0

// throwing errors

#undef THROW_ERROR

// accessing stack

#undef STACK_TOP
#undef STACK_END

#undef STACK_GET_TOP
#undef STACK_GET_END

#undef STACK_ACCESS
#undef STACK_ACCESS_TYPE
#undef STACK_ACCESS_TOP

// push

#undef STACK_PUSH_TYPE
#undef STACK_PUSH_2BACK
#undef STACK_SET_TYPE
#undef STACK_SET_TYPE_OFFSET

#undef STACK_PUSH_8
#undef STACK_PUSH_16
#undef STACK_PUSH_32
#undef STACK_PUSH_64

#undef STACK_PUSH_ADDR

// get

#undef STACK_GET_TYPE
#undef STACK_GET

#undef STACK_GET_U8
#undef STACK_GET_U16
#undef STACK_GET_U32
#undef STACK_GET_U64

#undef STACK_GET_I8
#undef STACK_GET_I16
#undef STACK_GET_I32
#undef STACK_GET_I64

#undef STACK_GET_F32
#undef STACK_GET_F64

#undef STACK_GET_BYTE

// pop

#undef STACK_POP_BYTES_UNSAFE
#undef STACK_POP_BYTES
#undef STACK_POP_TYPE
#undef STACK_POP_2BACK
#undef STACK_POP

#undef STACK_POP_8
#undef STACK_POP_16
#undef STACK_POP_32
#undef STACK_POP_64

// swap, dup & pull

#undef STACK_SWAP_TYPE
#undef STACK_DUP_TYPE
#undef STACK_PULL_TYPE

// stack modifying operations

#undef STACK_OPERATION_BINARY_ASSIGN
#undef STACK_OPERATION_UNARAY
#undef STACK_OPERATION_UNARAY_POSTFIX
#undef STACK_OPERATION_BINARY
#undef STACK_OPERATION_BINARY_FUNC
#undef STACK_OPERATION_BINARY_FUNC_ZERO_CHECK
#undef STACK_OPERATION_BINARY_ASSIGN_ZERO_CHECK

#endif

#undef WAVE_VM_STACK_INLINE_DEFINE
#undef WAVE_VM_STACK_INLINE_THROW_ERRORS
#undef WAVE_VM_SAFE_MODE
