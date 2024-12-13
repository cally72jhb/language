#define WAVE_VM_SAFE_MODE (0)
#define WAVE_VM_STACK_INLINE_DEFINE (1)
#define WAVE_VM_STACK_INLINE_THROW_ERRORS (0)
#include "language/runtime/wave_vm_stack_inline.h"

#ifndef WAVE_VM_NATIVE_FUNCTION
#define WAVE_VM_NATIVE_FUNCTION(package, function_name, parameter_size, statement) error_code CONCAT4(wave_vm_, package, _, function_name) (const byte* stack_start, const byte* stack_end, byte* stack, byte** out_stack);
#endif