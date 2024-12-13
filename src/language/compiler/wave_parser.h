#ifndef WAVE_LANGUAGE_PARSER
#define WAVE_LANGUAGE_PARSER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/compiler/wave_compiler_common.h"

#include "language/runtime/wave_vm.h"

// Parser Functions

error_code wave_compiler_parser_compile(wave_vm* vm, parse_token* tokenized_start, parse_token* tokenized_end, byte* data_stack_start, byte* data_stack_end);
error_code wave_compiler_parser_destroy(void);

#endif
