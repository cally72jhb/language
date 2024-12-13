#include "wave_vm_container.h"

#include "language/func/wave_debug.h"
#include "language/func/wave_math.h"

// Functions

#define BATCH_AMOUNT (32)

// fast vm execute function ("unsafe" mode, does not check for stack overflow, invalid function calls, ...)

#define WAVE_VM_EXECUTE_FUNCTION_NAME wave_vm_execute_entire_fast
#define WAVE_VM_SAFE_MODE (0)
#define WAVE_VM_INSTRUCTION_EXECUTION_COUNT WAVE_VM_EXECUTE_ALL
#include "wave_vm_inline.h"

#define WAVE_VM_EXECUTE_FUNCTION_NAME wave_vm_execute_batch_fast
#define WAVE_VM_SAFE_MODE (0)
#define WAVE_VM_INSTRUCTION_EXECUTION_COUNT BATCH_AMOUNT
#include "wave_vm_inline.h"

// safe vm execute function (check for stack overflow, valid function calls, ...)

#define WAVE_VM_EXECUTE_FUNCTION_NAME wave_vm_execute_entire_safe
#define WAVE_VM_SAFE_MODE (1)
#define WAVE_VM_INSTRUCTION_EXECUTION_COUNT WAVE_VM_EXECUTE_ALL
#include "wave_vm_inline.h"

#define WAVE_VM_EXECUTE_FUNCTION_NAME wave_vm_execute_batch_safe
#define WAVE_VM_SAFE_MODE (1)
#define WAVE_VM_INSTRUCTION_EXECUTION_COUNT BATCH_AMOUNT
#include "wave_vm_inline.h"

#undef BATCH_AMOUNT

// Native Functions

#define REGISTER_BUILTIN_FUNCTION(package, function_name, function_return_type, error, ...) REGISTER_FUNCTION(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, __VA_ARGS__)

#define REGISTER_BUILTIN_FUNCTION_0(package, function_name, function_return_type, error) REGISTER_FUNCTION_0(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error)
#define REGISTER_BUILTIN_FUNCTION_1(package, function_name, function_return_type, error, param1)                                                         REGISTER_FUNCTION_1(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1)
#define REGISTER_BUILTIN_FUNCTION_2(package, function_name, function_return_type, error, param1, param2)                                                 REGISTER_FUNCTION_2(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2)
#define REGISTER_BUILTIN_FUNCTION_3(package, function_name, function_return_type, error, param1, param2, param3)                                         REGISTER_FUNCTION_3(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3)
#define REGISTER_BUILTIN_FUNCTION_4(package, function_name, function_return_type, error, param1, param2, param3, param4)                                 REGISTER_FUNCTION_4(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3, param4)
#define REGISTER_BUILTIN_FUNCTION_5(package, function_name, function_return_type, error, param1, param2, param3, param4, param5)                         REGISTER_FUNCTION_5(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3, param4, param5)
#define REGISTER_BUILTIN_FUNCTION_6(package, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6)                 REGISTER_FUNCTION_6(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3, param4, param5, param6)
#define REGISTER_BUILTIN_FUNCTION_7(package, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7)         REGISTER_FUNCTION_7(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7)
#define REGISTER_BUILTIN_FUNCTION_8(package, function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7, param8) REGISTER_FUNCTION_8(vm, CONCAT4(wave_vm_, package, _, function_name), function_name, function_return_type, error, param1, param2, param3, param4, param5, param6, param7, param8)

error_code wave_vm_register_math_functions(wave_vm* vm) {
    REGISTER_BUILTIN_FUNCTION_1(math, f32_sin, WAVE_TYPE_F32, false, WAVE_TYPE_F32);
    REGISTER_BUILTIN_FUNCTION_1(math, f32_cos, WAVE_TYPE_F32, false, WAVE_TYPE_F32);
    REGISTER_BUILTIN_FUNCTION_1(math, f32_tan, WAVE_TYPE_F32, false, WAVE_TYPE_F32);
    REGISTER_BUILTIN_FUNCTION_2(math, f32_atan2, WAVE_TYPE_F32, false, WAVE_TYPE_F32, WAVE_TYPE_F32);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_register_string_functions(wave_vm* vm) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_register_file_functions(wave_vm* vm) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_register_debug_functions(wave_vm* vm) {
    REGISTER_BUILTIN_FUNCTION_1(debug, print, WAVE_TYPE_VOID, false, WAVE_TYPE_STR);
    REGISTER_BUILTIN_FUNCTION_1(debug, println, WAVE_TYPE_VOID, false, WAVE_TYPE_STR);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_vm_register_default_functions(wave_vm* vm) {
    RUN_ERROR_CODE_FUNCTION(wave_vm_register_math_functions, vm);
    RUN_ERROR_CODE_FUNCTION(wave_vm_register_string_functions, vm);
    RUN_ERROR_CODE_FUNCTION(wave_vm_register_file_functions, vm);
    RUN_ERROR_CODE_FUNCTION(wave_vm_register_debug_functions, vm);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

#undef REGISTER_FUNC
