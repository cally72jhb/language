#ifndef WAVE_LANGUAGE_DISASSEMBLER
#define WAVE_LANGUAGE_DISASSEMBLER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/runtime/wave_vm.h"

#include "language/compiler/wave_compiler.h"

// Typedefs

typedef error_code (*wave_disassembler_print_function)(str string, u32 length);

// Functions

error_code wave_disassemble(wave_vm* vm, wave_disassembler_print_function print_function);

#endif
