#ifndef WAVE_LANGUAGE_VM_CONTAINER
#define WAVE_LANGUAGE_VM_CONTAINER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/runtime/wave_vm.h"

// Defines

#define REGISTER_FUNCTION_LENGTH(vm, function, function_name, function_name_length, function_return_type, error, ...)   \
    do {                                                                                                                \
        wave_parameter function_parameters[] = { __VA_ARGS__ };                                                          \
        wave_vm_register_function(vm, (wave_native_function) {                                                            \
            .function_data = (wave_function) {                                                                           \
                .name = hash_bytes((byte*) function_name, function_name_length),                                        \
                .return_type = function_return_type,                                                                    \
                                                                                                                        \
                .parameters = function_parameters,                                                                      \
                .parameter_count = ARRAY_LENGTH(function_parameters),                                                   \
                                                                                                                        \
                .error_function = error                                                                                 \
            },                                                                                                          \
                                                                                                                        \
            .callback = function                                                                                        \
        });                                                                                                             \
    } while (0)

#define REGISTER_FUNCTION(vm, function, function_name, function_return_type, error, ...) REGISTER_FUNCTION_LENGTH(vm, function, #function_name, STRING_LENGTH(#function_name), function_return_type, error, __VA_ARGS__)

// no default value parameter macros

#define REGISTER_FUNCTION_0(vm, function, function_name, function_return_type, error) REGISTER_FUNCTION(vm, function, function_name, function_return_type, error)

#define REGISTER_FUNCTION_DEFAULT_VALUE_PART .has_default_value = false, .default_value = (union_number) { .value_u64 = 0 }

#define REGISTER_FUNCTION_1(vm, function, function_name, function_return_type, error, param1)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                 \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                \
    )

#define REGISTER_FUNCTION_2(vm, function, function_name, function_return_type, error, param1, param2)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                         \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                       \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                        \
    )

#define REGISTER_FUNCTION_3(vm, function, function_name, function_return_type, error, param1, param2, param3)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                 \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                               \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                               \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                \
    )

#define REGISTER_FUNCTION_4(vm, function, function_name, function_return_type, error, param1, param2, param3, param4)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                         \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                       \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                       \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                       \
        { .type = param4, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                        \
    )

#define REGISTER_FUNCTION_5(vm, function, function_name, function_return_type, error, param1, param2, param3, param4, param5)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                                 \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                               \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                               \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                               \
        { .type = param4, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                               \
        { .type = param5, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                                \
    )

#define REGISTER_FUNCTION_6(vm, function, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                                         \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                       \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                       \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                       \
        { .type = param4, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                       \
        { .type = param5, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                       \
        { .type = param6, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                                        \
    )

#define REGISTER_FUNCTION_7(vm, function, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                                                 \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param4, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param5, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param6, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                               \
        { .type = param7, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                                                \
    )

#define REGISTER_FUNCTION_8(vm, function, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7, param8)   \
    REGISTER_FUNCTION(vm, function, function_name, function_return_type, error,                                                                         \
        { .type = param1, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param2, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param3, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param4, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param5, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param6, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param7, REGISTER_FUNCTION_DEFAULT_VALUE_PART },                                                                                       \
        { .type = param8, REGISTER_FUNCTION_DEFAULT_VALUE_PART }                                                                                        \
    )

// Functions

error_code wave_vm_execute_entire_fast(wave_vm* vm);
error_code wave_vm_execute_batch_fast(wave_vm* vm);
error_code wave_vm_execute_entire_safe(wave_vm* vm);
error_code wave_vm_execute_batch_safe(wave_vm* vm);

// Native Functions

error_code wave_vm_register_math_functions(wave_vm* vm);
error_code wave_vm_register_string_functions(wave_vm* vm);
error_code wave_vm_register_file_functions(wave_vm* vm);
error_code wave_vm_register_debug_functions(wave_vm* vm);

error_code wave_vm_register_default_functions(wave_vm* vm);

#endif
