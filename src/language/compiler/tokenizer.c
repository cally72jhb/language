#include "tokenizer.h"

#include "common/constants.h"
#include "common/error_codes.h"

#include "common/data/string/hash.h"
#include "common/data/string/string.h"

#include "language/compiler/compiler.h"
#include "language/compiler/data/wave_compiler_common.h"

#include "language/runtime/wave_vm.h"

// static token list

typedef struct { wave_token token; cstr string; } token_table_entry;

static token_table_entry keyword_tokens[] = {
    #define KEYWORD_TOKENS (1)
    #define TOKEN_ENTRY(token_name, string_data, ...) (token_table_entry) { .token = CONCAT(WAVE_TOKEN_, token_name), .string = string_data },
    #include "language/compiler/data/wave_token_list_inline.h"
};

static token_table_entry operator_tokens[] = {
    #define OPERATOR_TOKENS (1)
    #define TOKEN_ENTRY(token_name, string_data, ...) (token_table_entry) { .token = CONCAT(WAVE_TOKEN_, token_name), .string = string_data },
    #include "language/compiler/data/wave_token_list_inline.h"
};

// error handling

#define TOKENIZER_RAISE_ERROR(function_name, message_format, ...) COMPILER_RAISE_ERROR(function_name, tokenizer.current_file_name, tokenizer.current_line, (tokenizer.current_row_start + 1) - tokenizer.current_line_start, message_format, __VA_ARGS__)

// token stack access

#define DATA_STACK_FITS_SIZE(size)                                                                                                                                          \
    do {                                                                                                                                                                    \
        if (tokenizer.data_stack_current + (size) > tokenizer.data_stack_end) {                                                                                             \
            tokenizer.data_stack_capacity += DATA_STACK_GROW_SIZE;                                                                                                          \
            if (tokenizer.vm->reallocate_memory((void**) &(tokenizer.data_stack_start), sizeof(byte) * tokenizer.data_stack_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) { \
                TOKENIZER_RAISE_ERROR("data_stack", "failed to reallocate data stack");                                                                                     \
            }                                                                                                                                                               \
        }                                                                                                                                                                   \
    } while (0)

#define TOKENIZER_PUSH_DATA_UNSAFE(value_type, token_data)          \
    do {                                                            \
        *((value_type*) tokenizer.data_stack_current) = token_data; \
        tokenizer.data_stack_current += sizeof(value_type);         \
    } while (0)

// source access

#define NEXT_CHAR() do { source++; } while (0)
#define GET_CHAR() *source

// tokenization interface

static struct {
    wave_vm* vm;

    parse_token* token_stack_start;
    parse_token* token_stack_end;
    parse_token* token_stack_current;
    u32 token_stack_capacity;

    byte* data_stack_start;
    byte* data_stack_end;
    byte* data_stack_current;
    u32 data_stack_capacity;

    str current_file_name;
    u32 current_line;

    str current_line_start;
    str current_row_start;
} tokenizer;

// Tokenizer Functions

static void push_token(wave_token token, bool has_data) {
    if (tokenizer.token_stack_current + sizeof(parse_token) > tokenizer.token_stack_end) {
        tokenizer.token_stack_capacity += TOKEN_STACK_GROW_SIZE;
        if (tokenizer.vm->reallocate_memory((void**) &(tokenizer.token_stack_start), sizeof(parse_token) * tokenizer.token_stack_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            TOKENIZER_RAISE_ERROR("token_stack", "failed to reallocate token stack");
            return;
        }
    }

    *tokenizer.token_stack_current = (parse_token) {
        .token = token,
        .data_index = has_data ? (u32) (tokenizer.data_stack_current - tokenizer.data_stack_start) : 0,

        .line = tokenizer.current_line,
        .row = (tokenizer.current_row_start + 1) - tokenizer.current_line_start
    };

    tokenizer.token_stack_current++;
}

static void skip_whitespaces(str source, str* out_source) {
    while (char_is_whitespace(GET_CHAR())) {
        if (GET_CHAR() == '\n') {
            tokenizer.current_line_start = source + 1;
            tokenizer.current_line++;
        }

        NEXT_CHAR();
    }

    *out_source = source;
}

static void skip_comments(str source, str* out_source) {
    if (GET_CHAR() == '/') {
        NEXT_CHAR();

        if (GET_CHAR() == '/') { // single line comment
            NEXT_CHAR();

            while (GET_CHAR() != '\n' && GET_CHAR() != '\0') {
                NEXT_CHAR();
            }

            if (GET_CHAR() == '\0') {
                goto skip_comments_end;
            }

            tokenizer.current_line_start = source + 1;
            tokenizer.current_line++;
        } else if (GET_CHAR() == '*') { // multi-line/inline comment
            NEXT_CHAR();

            while (true) {
                if (GET_CHAR() == '\0') {
                    goto skip_comments_end;
                } else if (GET_CHAR() == '\n') {
                    tokenizer.current_line_start = source + 1;
                    tokenizer.current_line++;
                }

                if (source[0] == '*' && source[1] == '/') {
                    NEXT_CHAR();
                    break;
                }

                NEXT_CHAR();
            }

            NEXT_CHAR();
        }
    }

    skip_comments_end: {}

    *out_source = source;
}

static void tokenize_number(str source, str* out_source) {
    u64 integer_result = 0;
    f32 f32_representation = 0.0F;
    f64 f64_representation = 0.0;

    if (GET_CHAR() == '0') {
        NEXT_CHAR();

        if (GET_CHAR() == 'x') {
            NEXT_CHAR();

            do {
                char character = GET_CHAR();
                if (char_is_digit(character)) {
                    integer_result = integer_result * 16 + (character - '0');
                } else if (character >= 'a' && character <= 'f') {
                    integer_result = integer_result * 16 + (character - 'a' + 10);
                } else if (character >= 'A' && character <= 'F') {
                    integer_result = integer_result * 16 + (character - 'A' + 10);
                } else if (character == '_') {
                    NEXT_CHAR();
                    continue;
                } else {
                    break;
                }

                NEXT_CHAR();
            } while (true);
        } else if (GET_CHAR() == 'b') {
            NEXT_CHAR();

            while (GET_CHAR() == '0' || GET_CHAR() == '1' || GET_CHAR() == '_') {
                if (GET_CHAR() == '_') {
                    NEXT_CHAR();
                    continue;
                }

                integer_result = (integer_result << 1) | (GET_CHAR() - '0');
                NEXT_CHAR();
            }
        } else if (char_is_digit(GET_CHAR()) || GET_CHAR() == '_') {
            goto wave_compile_bytecode_parse_number_base10;
        } else {
            integer_result = 0;
        }
    } else {
        wave_compile_bytecode_parse_number_base10: {}

        while (char_is_digit(GET_CHAR()) || GET_CHAR() == '_') {
            if (GET_CHAR() == '_') {
                NEXT_CHAR();
                continue;
            }

            integer_result = integer_result * 10 + (u64) (GET_CHAR() - '0');
            NEXT_CHAR();
        }
    }

    if (GET_CHAR() == '.') {
        NEXT_CHAR();
        u32 divisor = 10;

        while (char_is_digit(GET_CHAR()) || GET_CHAR() == '_') {
            if (GET_CHAR() == '_') {
                NEXT_CHAR();
                continue;
            }

            f32_representation += ((f32) (GET_CHAR() - '0')) / ((f32) divisor);
            f64_representation += ((f64) (GET_CHAR() - '0')) / ((f64) divisor);
            divisor *= 10;

            NEXT_CHAR();
        }

        f32_representation += (f32) integer_result;
        f64_representation += (f64) integer_result;

        DATA_STACK_FITS_SIZE(sizeof(wave_float));
        wave_float value = (wave_float) { .f32 = f32_representation, .f64 = f64_representation };
        push_token(WAVE_TOKEN_VALUE_FLOAT, true);
        TOKENIZER_PUSH_DATA_UNSAFE(wave_float, value);
    } else {
        DATA_STACK_FITS_SIZE(sizeof(u64));
        push_token(WAVE_TOKEN_VALUE_INTEGER, true);
        TOKENIZER_PUSH_DATA_UNSAFE(u64, integer_result);
    }

    *out_source = source;
}

static bool tokenize_string(str source, str* out_source) {
    DATA_STACK_FITS_SIZE(sizeof(u32));

    byte* temp_data_stack = tokenizer.data_stack_current;
    push_token(WAVE_TOKEN_VALUE_STR, true);
    TOKENIZER_PUSH_DATA_UNSAFE(u32, 0);

    u32 length = 0;

    wave_compiler_builtin_tokenize_string: {}
    do {
        if (GET_CHAR() == '\\') {
            NEXT_CHAR();
            char character = '\0';
            switch (GET_CHAR()) {
                case ('0'): { character = '\0'; break; }
                case ('n'): { character = '\n'; break; }
                case ('r'): { character = '\r'; break; }
                case ('t'): { character = '\t'; break; }
                case ('v'): { character = '\v'; break; }

                case ('\\'): { character = '\\'; break; }

                case ('\''): { character = '\''; break; }
                case ('\"'): { character = '\"'; break; }

                case ('b'): {
                    u8 character_temp = 0;
                    while (GET_CHAR() != '\'' && (GET_CHAR() == '0' || GET_CHAR() == '1' || GET_CHAR() == '_')) {
                        if (GET_CHAR() == '_') {
                            continue;
                        }

                        character_temp = (character_temp << 1) & (GET_CHAR() - '0');
                        NEXT_CHAR();
                    }

                    character = *((char*) &character_temp);
                    break;
                }

                default: {
                    character = GET_CHAR();
                    break;
                }
            }

            TOKENIZER_PUSH_DATA_UNSAFE(char, character);
            length++;
            continue;
        } if (GET_CHAR() == '\n' || GET_CHAR() == '\r' || GET_CHAR() == '\t' || GET_CHAR() == '\v') {
            NEXT_CHAR();
            continue;
        }

        TOKENIZER_PUSH_DATA_UNSAFE(char, GET_CHAR());
        NEXT_CHAR();
        length++;
    } while (GET_CHAR() != '"' && GET_CHAR() != '\0');
    if (GET_CHAR() == '\0') {
        TOKENIZER_RAISE_ERROR("tokenize_string", "sudden end of file, encountered null character");
        return false;
    }

    // skip whitespace and comments in between concatenated string ("abc" "def" -> "abcdef")

    NEXT_CHAR();
    while (char_is_whitespace(GET_CHAR()) || GET_CHAR() == '\\') {
        skip_whitespaces(source, &source);
        skip_comments(source, &source);
    }

    if (GET_CHAR() == '"') {
        NEXT_CHAR();
        goto wave_compiler_builtin_tokenize_string;
    } else {
        *((u32*) temp_data_stack) = length;
    }

    *out_source = source;

    return true;
}

static void tokenize_character(str source, str* out_source) {
    DATA_STACK_FITS_SIZE(sizeof(u64));

    char character = '\0';
    if (GET_CHAR() == '\\') {
        NEXT_CHAR();

        switch (GET_CHAR()) {
            case ('0'): { character = '\0'; break; }
            case ('n'): { character = '\n'; break; }
            case ('r'): { character = '\r'; break; }
            case ('t'): { character = '\t'; break; }
            case ('v'): { character = '\v'; break; }

            case ('\\'): { character = '\\'; break; }

            case ('\''): { character = '\''; break; }
            case ('\"'): { character = '\"'; break; }

            case ('b'): {
                u8 character_temp = 0;
                while (GET_CHAR() != '\'' && (GET_CHAR() == '0' || GET_CHAR() == '1' || GET_CHAR() == '_')) {
                    if (GET_CHAR() == '_') {
                        continue;
                    }

                    character_temp = (character_temp << 1) & (GET_CHAR() - '0');
                    NEXT_CHAR();
                }

                character = *((char*) &character_temp);
                goto wave_compile_bytecode_parse_character_end;
            }

            default: {
                character = GET_CHAR();
                break;
            }
        }
    } else {
        character = GET_CHAR();
    }

    NEXT_CHAR();

    wave_compile_bytecode_parse_character_end: {}

    push_token(WAVE_TOKEN_VALUE_INTEGER, true);
    TOKENIZER_PUSH_DATA_UNSAFE(u64, (u64) *((u8*) (&character)));

    *out_source = source;
}

static void tokenize_identifier(str source, str* out_source) {
    DATA_STACK_FITS_SIZE(sizeof(wave_identifier));

    u32 length = 0;
    str temp = source;

    do {
        NEXT_CHAR();
        length++;
    } while (wave_compiler_builtin_char_is_namespace(GET_CHAR()));

    push_token(WAVE_TOKEN_IDENTIFIER, true);
    wave_identifier identifier = (wave_identifier) { .hash = hash_bytes((byte*) temp, length), .source_pointer = temp };
    TOKENIZER_PUSH_DATA_UNSAFE(wave_identifier, identifier);

    *out_source = source;
}

static wave_token tokenize_keyword(str source, str* out_source) {
    str temp = NULL;
    wave_token token = WAVE_TOKEN_INVALID;

    for (u32 i = 0; i < ARRAY_LENGTH(keyword_tokens); i++) {
        temp = source;
        cstr keyword = keyword_tokens[i].string;

        while (*temp == *keyword) {
            temp++;
            keyword++;

            if (*keyword == '\0' && !char_is_letter(*temp) && !char_is_digit(*temp)) {
                token = keyword_tokens[i].token;
                source = temp;
                break;
            } else if (*temp == '\0') {
                goto tokenize_keyword_end;
            }
        }

        if (token != WAVE_TOKEN_INVALID) {
            goto tokenize_keyword_end; // token found
        }
    }

    tokenize_keyword_end: {}

    *out_source = source;

    return token;
}

static wave_token tokenize_operator(str source, str* out_source) {
    str temp = NULL;
    wave_token token = WAVE_TOKEN_INVALID;

    for (u32 i = 0; i < ARRAY_LENGTH(operator_tokens); i++) {
        temp = source;
        cstr keyword = operator_tokens[i].string;

        while (*temp == *keyword) {
            temp++;
            keyword++;

            if (*keyword == '\0') {
                token = operator_tokens[i].token;
                source = temp;
                break;
            } else if (*temp == '\0') {
                goto tokenize_operator_end;
            }
        }

        if (token != WAVE_TOKEN_INVALID) {
            goto tokenize_operator_end; // token found
        }
    }

    tokenize_operator_end: {}

    *out_source = source;

    return token;
}

error_code wave_compiler_tokenize(wave_vm* vm, str source, parse_token** out_tokenized_start, parse_token** out_tokenized_end, byte** out_data_stack_start, byte** out_data_stack_end) {
    tokenizer.vm = vm;

    const wave_memory_allocation_function allocate_memory = tokenizer.vm->allocate_memory;
    const wave_memory_reallocation_function reallocate_memory = tokenizer.vm->reallocate_memory;

    // token stack

    parse_token* token_stack_start = NULL;
    u32 token_stack_capacity = TOKEN_STACK_GROW_SIZE;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &token_stack_start, sizeof(parse_token) * token_stack_capacity);

    tokenizer.token_stack_start    = token_stack_start;
    tokenizer.token_stack_end      = token_stack_start + token_stack_capacity;
    tokenizer.token_stack_current  = token_stack_start;
    tokenizer.token_stack_capacity = token_stack_capacity;

    // data stack

    byte* data_stack_start = NULL;
    u32 data_stack_capacity = DATA_STACK_GROW_SIZE;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &data_stack_start, sizeof(parse_token) * data_stack_capacity);

    tokenizer.data_stack_start    = data_stack_start;
    tokenizer.data_stack_end      = data_stack_start + data_stack_capacity;
    tokenizer.data_stack_current  = data_stack_start;
    tokenizer.data_stack_capacity = data_stack_capacity;

    tokenizer.current_file_name = "undefined";
    tokenizer.current_line = 1;

    tokenizer.current_line_start = source;
    tokenizer.current_row_start = source;

    // parsing tokens

    while (GET_CHAR() != '\0') {
        while (char_is_whitespace(GET_CHAR()) || GET_CHAR() == '\\') {
            skip_whitespaces(source, &source);
            skip_comments(source, &source);
        }

        if (GET_CHAR() == '\0') {
            break;
        }

        wave_token token = WAVE_TOKEN_INVALID;

        tokenizer.current_row_start = source;

        // tokenizing numbers

        if (char_is_digit(GET_CHAR())) {
            tokenize_number(source, &source);
            continue;
        }

        // tokenizing strings

        if (GET_CHAR() == '"') {
            NEXT_CHAR();
            if (!tokenize_string(source, &source)) {
                return ERROR_CODE_EXECUTION_FAILED;
            }

            continue;
        }

        // tokenizing characters

        if (GET_CHAR() == '\'') {
            NEXT_CHAR();
            tokenize_character(source, &source);
            if (GET_CHAR() != '\'') {
                return ERROR_CODE_LANGUAGE_COMPILER_SUDDEN_END_OF_FILE;
            }

            continue;
        }

        // tokenizing keywords

        token = tokenize_keyword(source, &source);
        if (token != WAVE_TOKEN_INVALID) {
            push_token(token, false);
            continue;
        } else if (GET_CHAR() == '\0') {
            return ERROR_CODE_LANGUAGE_COMPILER_SUDDEN_END_OF_FILE;
        }

        // tokenizing variable and function names (identifiers)

        if (!char_is_digit(GET_CHAR()) && wave_compiler_builtin_char_is_namespace(GET_CHAR())) {
            tokenize_identifier(source, &source);
            continue;
        }

        // tokenizing operators

        token = tokenize_operator(source, &source);
        if (token != WAVE_TOKEN_INVALID) {
            push_token(token, false);
            continue;
        } else if (GET_CHAR() == '\0') {
            return ERROR_CODE_LANGUAGE_COMPILER_SUDDEN_END_OF_FILE;
        }

        // if this executes something went wrong

        return ERROR_CODE_LANGUAGE_COMPILER_UNKNOWN_CHARACTER;
    }

    tokenizer.current_line_start = source;
    tokenizer.current_row_start = source;
    push_token(WAVE_TOKEN_FILE_END, false);

    // shrink the allocated stacks to fit

    u32 token_stack_length = tokenizer.token_stack_current - tokenizer.token_stack_start;
    u32 data_stack_length  = tokenizer.data_stack_current  - tokenizer.data_stack_start;
    RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(tokenizer.token_stack_start), sizeof(parse_token) * (token_stack_length == 0 ? 0 : token_stack_length - 1));
    RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(tokenizer.data_stack_start),  sizeof(byte)        * (data_stack_length == 0 ? 0 : data_stack_length - 1));

    // output result

    *out_tokenized_start = tokenizer.token_stack_start;
    *out_tokenized_end = tokenizer.token_stack_end;

    *out_data_stack_start = tokenizer.data_stack_start;
    *out_data_stack_end = tokenizer.data_stack_end;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;

}

#undef TOKEN_STACK_FITS_SIZE
#undef DATA_STACK_FITS_SIZE
#undef TOKENIZER_PUSH_TOKEN_UNSAFE
#undef TOKENIZER_PUSH_DATA_UNSAFE
#undef TOKENIZER_PUSH_TOKEN

#undef NEXT_CHAR
#undef GET_CHAR

error_code wave_compiler_tokenizer_destroy(void) {
    const wave_memory_deallocation_function deallocate_memory = tokenizer.vm->deallocate_memory;

    #define TOKENIZER_DEALLOCATE(pointer) do { if (pointer != NULL) { RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void*) pointer); pointer = NULL; } } while (0)

    TOKENIZER_DEALLOCATE(tokenizer.token_stack_start);
    TOKENIZER_DEALLOCATE(tokenizer.data_stack_start);

    #undef TOKENIZER_DEALLOCATE

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
