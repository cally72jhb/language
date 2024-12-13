#ifndef WAVE_LANGUAGE_TOKENIZER
#define WAVE_LANGUAGE_TOKENIZER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/compiler/wave_compiler_common.h"

#include "language/runtime/wave_vm.h"

// Tokenizer Functions

static void push_token(wave_token token, bool has_data);

static void skip_whitespaces(str source, str* out_source);
static void skip_comments(str source, str* out_source);
static void tokenize_number(str source, str* out_source);
static bool tokenize_string(str source, str* out_source);
static void tokenize_character(str source, str* out_source);
static void tokenize_identifier(str source, str* out_source);
static wave_token tokenize_keyword(str source, str* out_source);
static wave_token tokenize_operator(str source, str* out_source);

error_code wave_compiler_tokenize(wave_vm* vm, str source, parse_token** out_tokenized_start, parse_token** out_tokenized_end, byte** out_data_stack_start, byte** out_data_stack_end);
error_code wave_compiler_tokenizer_destroy(void);

#endif
