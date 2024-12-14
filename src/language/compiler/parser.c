#include "parser.h"

#include "common/debug.h"

#include "common/data/string/hash.h"
#include "common/data/string/string.h"

#include "language/wave_common.h"
#include "language/wave_limits.h"
#include "language/wave_opcodes.h"

#include "language/compiler/compiler.h"
#include "language/compiler/data/wave_compiler_common.h"
#include "language/compiler/data/wave_precedence.h"
#include "language/compiler/data/wave_type.h"

// Parser Functions

static void parse_literal(wave_type expression_type, bool can_assign);
static void parse_number(wave_type expression_type, bool can_assign);
static void parse_string(wave_type expression_type, bool can_assign);

static void parse_identifier(wave_type expression_type, bool can_assign);

static void parse_dot(wave_type expression_type, bool can_assign);

static void parse_unary(wave_type expression_type, bool can_assign);
static void parse_binary(wave_type expression_type, bool can_assign);
static void parse_grouping(wave_type expression_type, bool can_assign);

static void parse_type_conversion(wave_type expression_type, bool can_assign);

// Typedefs

typedef struct {
    string_hash name;
    wave_type type;
} variable;

typedef struct {
    string_hash name;
    wave_type type;

    bool initialized;

    u16 offset;
    u32 depth;
} wave_local;

typedef struct {
    string_hash name;
    wave_type type;

    bool initialized;

    u16 offset;
} wave_global;

typedef struct {
    string_hash name;
    wave_type type;
    u16 offset;

    union_number default_value;
} parse_parameter;

typedef struct {
    string_hash name;
    u32 branch_offset;
} parse_label;

typedef struct {
    wave_function function_data;
    u16 locals_size;
    u32 branch_offset;

    bool initialized;

    bool inline_function;
} parse_function; // extends @wave_function
#define PARSE_FUNCTION_NULL                 \
    (parse_function) {                      \
        .function_data = (wave_function) {  \
            .name = 0,                      \
            .return_type = WAVE_TYPE_NONE,  \
            .parameters = NULL,             \
            .parameter_count = 0,           \
            .error_function = false         \
        },                                  \
                                            \
        .locals_size = 0,                   \
        .branch_offset = 0,                 \
        .initialized = false,               \
        .inline_function = false,           \
    }

typedef enum {
    PATCH_HOLE_TYPE_NONE,

    PATCH_HOLE_TYPE_FUNCTION_CALL, // patches function calls made to uninitialized functions
    PATCH_HOLE_TYPE_FUNCTION_REFERENCE, // patches variables that have been set to a function reference that has not yet been initialized
    PATCH_HOLE_TYPE_JUMP, // patches jump statements that do not yet have a branch offset

    PATCH_HOLE_TYPE_MAX
} PATCH_HOLE_TYPES;
typedef byte patch_hole_type; // PATCH_HOLE_TYPES

typedef struct {
    patch_hole_type type;
    u32 bytecode_index;

    union {
        string_hash identifier;
    } patch_hole_data;
} patch_hole;

typedef enum {
    COMPILER_SCOPE_GLOBAL = 0,
    COMPILER_SCOPE_FUNCTION,
    COMPILER_SCOPE_ENUMERATION,
    COMPILER_SCOPE_STRUCTURE,
    COMPILER_SCOPE_UNION,
    COMPILER_SCOPE_SWITCH
} parser_scope;

// parsing interfaces

static parsing_function parse_rule_function_table[] = {
    [PARSE_RULE_FUNC_LITERAL]  = parse_literal,
    [PARSE_RULE_FUNC_NUMBER]   = parse_number,
    [PARSE_RULE_FUNC_STRING]   = parse_string,

    [PARSE_RULE_FUNC_IDENTIFIER] = parse_identifier,

    [PARSE_RULE_FUNC_DOT] = parse_dot,

    [PARSE_RULE_FUNC_UNARY] = parse_unary,
    [PARSE_RULE_FUNC_BINARY] = parse_binary,
    [PARSE_RULE_FUNC_GROUPING] = parse_grouping,

    [PARSE_RULE_FUNC_CAST] = parse_type_conversion
};

static struct {
    wave_vm* vm;

    // tokens & token data

    parse_token* tokenized_start;
    parse_token* tokenized_end;
    parse_token* tokenized_current;

    parse_token* tokenized_stored;

    byte* data_stack_start;
    byte* data_stack_end;

    str current_file_name;
    u32 current_line;
    u32 current_row;

    u32 previous_line;
    u32 previous_row;

    // compiled bytecode result

    byte* bytecode_start;
    byte* bytecode_end;
    byte* bytecode_current;
    u32 bytecode_capacity;

    // parsing interfaces

    parse_token current;
    parse_token previous;

    // global variables

    wave_global* globals;
    u16 globals_count;
    u16 globals_capacity;
    u16 globals_offset; // the next free byte in the globals array

    // functions

    parse_function entrypoint_function;
    bool current_scope_is_entrypoint_function;

    parse_function* current_function;

    parse_function* functions; // TODO: hash table
    u32 functions_count;
    u32 functions_capacity;

    u32* extern_functions;
    u32 extern_functions_capacity;
    u32 extern_functions_count;

    // patch holes

    patch_hole* patch_holes;
    u32 patch_hole_capacity;
    u32 patch_hole_count;
} parser;

struct {
    u32 scope_depth;

    // global variables

    u16* accessed_globals; // globals that have been set to a value referencing an object on the stack inside the scope of the function
    u16 accessed_globals_capacity;
    u16 accessed_globals_count;

    // local variables

    wave_local* locals;
    u16 locals_capacity;
    u16 locals_count;
    u16 locals_offset; // the next free byte in the locals array

    // labels

    parse_label* labels;
    u32 label_capacity;
    u32 label_count;
} function_parser; // TODO: merge with @parser

// Defines

#define PARSER_GET_DATA(type, offset) (*((type*) (parser.data_stack_start + (offset))))

#define PARSER_EXPECT_RETURN(return_expression, token, parse_function, message_format, ...) do { if (!parser_consume(token)) { PARSER_RAISE_ERROR(parse_function, message_format, __VA_ARGS__); return return_expression; } } while (0)
#define PARSER_EXPECT(token, parse_function, message_format, ...) PARSER_EXPECT_RETURN(, token, parse_function, message_format, __VA_ARGS__)

// error handling

#define PARSER_RAISE_WARNING(function_name, message_format, ...) COMPILER_RAISE_WARNING(function_name, parser.current_file_name, parser.current_line, parser.current_row, message_format, __VA_ARGS__)

#define PARSER_RAISE_ERROR(function_name, message_format, ...) COMPILER_RAISE_ERROR(function_name, parser.current_file_name, parser.current_line, parser.current_row, message_format, __VA_ARGS__)
#define PARSER_RAISE_ERROR_PREV(function_name, message_format, ...) COMPILER_RAISE_ERROR(function_name, parser.current_file_name, parser.previous_line, parser.previous_row, message_format, __VA_ARGS__)
#define PARSER_RAISE_ERROR_AT(function_name, message_format, line, row, ...) COMPILER_RAISE_ERROR(function_name, parser.current_file_name, line, row, message_format, __VA_ARGS__)

// helper

#define STACK_HELPER_PUSH(pointer, value, type_size, capacity, count, grow_size, function, error_message, ...)                      \
    do {                                                                                                                            \
        if (count + 1 >= (capacity)) {                                                                                              \
            capacity += grow_size;                                                                                                  \
            if (parser.vm->reallocate_memory((void**) &(pointer), (type_size) * (capacity)) != ERROR_CODE_EXECUTION_SUCCESSFUL) {   \
                PARSER_RAISE_ERROR(function, error_message);                                                                        \
                return __VA_ARGS__;                                                                                                 \
            }                                                                                                                       \
        }                                                                                                                           \
                                                                                                                                    \
        pointer[count] = value;                                                                                                     \
        count++;                                                                                                                    \
    } while (0)

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

// Parser Functions

// scope

static void parser_begin_scope(void);
static void parser_end_scope(void);

// quick access

static parse_token parser_peek(i32 offset);

static void parser_advance(void);
static void parser_reverse(void);
static void parser_set(parse_token previous, parse_token current);

static bool parser_consume(wave_token token);
static bool parser_match(wave_token token);

static void parser_update_position(void);

// emit

static void emit_byte(byte value);
static void emit_bytes(byte value1, byte value2);
static void emit_u8(u8 value);
static void emit_u16(u16 value);
static void emit_u32(u32 value);
static void emit_u64(u64 value);

// complex parse

static void parse_expression(wave_type parent_expression_type); // any combination of arithmetic operations on numbers and/or variables
static void parse_declaration(void); // top level only: functions, global variables, the entrypoint ...
static void parse_statement(void); // parses statements and variable declarations
static void parse_block(void); // a separate scope that consists of zero or more statements

// parse statement

static void parse_expression_statement(void);

static void parse_variable_initializer_statement(void);
static void parse_variable_string_initializer_statement(void);

static void parse_function_call_statement(bool reference_function_call, wave_type* out_return_type);

static void parse_if_statement(void);
static void parse_do_statement(void);
static void parse_while_statement(void);
static void parse_for_statement(void);
static void parse_switch_statement(void);

static void parse_return_statement(void);
static void parse_exit_statement(void);

static void parse_label_statement(void);

// parse declaration

static void parse_function_parameters(parse_parameter** out_parameters, bool* out_function_forward_declared);
static void parse_function_body(str function_name_source_pointer, u32 function_start_line, u32 function_start_row, const parse_parameter* parameters);

static void parse_function_declaration(void);
static void parse_entrypoint_declaration(void);

static void parse_global_variable_declaration(void);

static void parse_local_variable_declaration(void);

static void parse_enum_declaration(void);
static void parse_struct_declaration(void);
static void parse_union_declaration(void);

static void parse_precedence(wave_type expression_type, parsing_precedence precedence); // precedence parsing for expression statements

// variables

static wave_local* add_local(wave_type type, string_hash name, bool initialized);
static u32 resolve_local(string_hash name, wave_local* out_variable);
static void add_global(wave_type type, string_hash name);
static bool resolve_global(string_hash name, wave_global* out_variable);

static bool resolve_variable(string_hash name, wave_type* out_type);
static bool emit_variable(string_hash name, bool evaluate, bool assign_expression);

// functions

static bool function_is_defined(string_hash name);
static bool resolve_function(string_hash name, parse_function* out_function);

// other

static bool is_function_modifier(wave_token token);

// Parser Functions

// scope

static void parser_begin_scope(void) {
    function_parser.scope_depth++;
}

static void parser_end_scope(void) {
    function_parser.scope_depth--;

    while (function_parser.locals_count > 0 && function_parser.locals[function_parser.locals_count - 1].depth > function_parser.scope_depth) { // only pop the variables of the current scope off the stack
        switch (function_parser.locals[function_parser.locals_count - 1].type) { // TODO: optimize using @OPCODE_POP_N
            case WAVE_TYPE_U8:
            case WAVE_TYPE_I8: {
                emit_byte(OPCODE_POP_8);
                break;
            }

            case WAVE_TYPE_U16:
            case WAVE_TYPE_I16: {
                emit_byte(OPCODE_POP_16);
                break;
            }

            case WAVE_TYPE_U32:
            case WAVE_TYPE_I32:
            case WAVE_TYPE_F32:

            case WAVE_TYPE_FUNC: {
                emit_byte(OPCODE_POP_32);
                break;
            }

            case WAVE_TYPE_U64:
            case WAVE_TYPE_I64:
            case WAVE_TYPE_F64: {
                emit_byte(OPCODE_POP_64);
                break;
            }

            case WAVE_TYPE_STR:
            case WAVE_TYPE_ARR:

            case WAVE_TYPE_ENUM:
            case WAVE_TYPE_STRUCT: {
                emit_byte(OPCODE_POP_FREE);
                break;
            }

            default: {
                PARSER_RAISE_ERROR("parser_end_scope", "encountered unknown type of local variable while attempting to pop it off the stack");
                return;
            }
        }

        function_parser.locals_count--;
    }
}

// quick access

static parse_token parser_peek(i32 offset) {
    return *(parser.tokenized_current + offset);
}

static void parser_advance(void) { // TODO: add out of bound check?
    if (parser.tokenized_current >= parser.tokenized_end) {
        PARSER_RAISE_ERROR("parser_advance", "unexpected: left the bounds fo the tokenized source");
        return;
    }

    parser.previous = parser.current;
    parser.current = *parser.tokenized_current;
    parser.tokenized_current++;

    parser_update_position();
}

static void parser_reverse(void) {
    if (parser.tokenized_current <= parser.tokenized_start) {
        PARSER_RAISE_ERROR("parser_reverse", "unexpected: left the bounds fo the tokenized source");
        return;
    }

    parser.current = parser.previous;
    parser.previous = *(parser.tokenized_current - 2);
    parser.tokenized_current--;

    parser_update_position();
}

static void parser_set(parse_token previous, parse_token current) {
    parser.previous = previous;
    parser.current = current;
}

static bool parser_consume(wave_token token) {
    if (parser.current.token == token) {
        parser_advance();
        return true;
    }

    return false;
}

static bool parser_match(wave_token token) {
    if (parser.current.token != token) {
        return false;
    }

    parser_advance();
    return true;
}

static void parser_update_position(void) {
    parser.previous_line = parser.previous.line;
    parser.previous_row = parser.previous.row;

    parser.current_line = parser.current.line;
    parser.current_row = parser.current.row;
}

// emit

#define BYTECODE_FITS_SIZE(size)                                                                                                                                \
    do {                                                                                                                                                        \
        if (parser.bytecode_current + (size) > parser.bytecode_end) {                                                                                           \
            parser.bytecode_capacity += BYTECODE_STACK_GROW_SIZE;                                                                                               \
            if (parser.vm->reallocate_memory((void**) &(parser.bytecode_start), sizeof(byte) * parser.bytecode_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {  \
                PARSER_RAISE_ERROR("bytecode", "failed to reallocate bytecode");                                                                                \
                return;                                                                                                                                         \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
    } while (0)

#define BYTECODE_PUSH_DATA_UNSAFE(type, data)       \
    do {                                            \
        *((type*) parser.bytecode_current) = data;  \
        parser.bytecode_current += sizeof(data);    \
    } while (0)

static void emit_byte(byte value) {
    BYTECODE_FITS_SIZE(sizeof(byte));
    BYTECODE_PUSH_DATA_UNSAFE(byte, value);
}

static void emit_bytes(byte value1, byte value2) {
    BYTECODE_FITS_SIZE(sizeof(byte) * 2);
    BYTECODE_PUSH_DATA_UNSAFE(byte, value1);
    BYTECODE_PUSH_DATA_UNSAFE(byte, value2);
}

static void emit_u8(u8 value)   { BYTECODE_FITS_SIZE(sizeof(u8));  BYTECODE_PUSH_DATA_UNSAFE(u8, value);  }
static void emit_u16(u16 value) { BYTECODE_FITS_SIZE(sizeof(u16)); BYTECODE_PUSH_DATA_UNSAFE(u16, value); }
static void emit_u32(u32 value) { BYTECODE_FITS_SIZE(sizeof(u32)); BYTECODE_PUSH_DATA_UNSAFE(u32, value); }
static void emit_u64(u64 value) { BYTECODE_FITS_SIZE(sizeof(u64)); BYTECODE_PUSH_DATA_UNSAFE(u64, value); }

#undef BYTECODE_FITS_SIZE
#undef BYTECODE_PUSH_DATA_UNSAFE

// complex parse

static void parse_expression(wave_type parent_expression_type) {
    parse_precedence(parent_expression_type, PRECEDENCE_ASSIGNMENT);
    DEBUG_INFO("line: %u, row: %u", parser.current_line, parser.current_row);
    if (compiler_has_error()) {
        return;
    }
}

static void parse_declaration(void) {
    if (parser.current.token == WAVE_TOKEN_KEYWORD_FUNC || is_function_modifier(parser.current.token)) {
        parse_function_declaration();
    } else if (parser.current.token == WAVE_TOKEN_KEYWORD_GLOBAL) {
        parser_consume(WAVE_TOKEN_KEYWORD_GLOBAL);
        parse_global_variable_declaration();
    } else if (token_get_wave_type(parser.current.token) != WAVE_TYPE_VOID && token_get_wave_type(parser.current.token) != WAVE_TYPE_NONE) {
        parse_global_variable_declaration();
    } else if (parser.current.token == WAVE_TOKEN_KEYWORD_ENTRYPOINT) {
        parse_entrypoint_declaration();
    }

    if (compiler_has_error()) {
        return;
    }
}

static void parse_statement(void) {
    wave_token token = parser.current.token;
    switch (token) {
        case WAVE_TOKEN_KEYWORD_IF:     { parser_advance(); parse_if_statement(); break; }
        case WAVE_TOKEN_KEYWORD_DO:     { parser_advance(); parse_do_statement(); break; }
        case WAVE_TOKEN_KEYWORD_WHILE:  { parser_advance(); parse_while_statement(); break; }
        case WAVE_TOKEN_KEYWORD_FOR:    { parser_advance(); parse_for_statement(); break; }
        case WAVE_TOKEN_KEYWORD_SWITCH: { parser_advance(); parse_switch_statement(); break; }

        case WAVE_TOKEN_KEYWORD_RETURN: {
            parser_advance();
            if (parser.current_scope_is_entrypoint_function) {
                parse_exit_statement();
            } else {
                parse_return_statement();
            }

            break;
        }

        case WAVE_TOKEN_KEYWORD_EXIT: {
            parser_advance();
            parse_exit_statement();
            break;
        }

        case WAVE_TOKEN_KEYWORD_LABEL: { parser_advance(); parse_label_statement(); break; }

        case WAVE_TOKEN_KEYWORD_GLOBAL: {
            parser_advance();
            parse_global_variable_declaration();
            break;
        }

        case WAVE_TOKEN_IDENTIFIER: {
            parser_advance();
            parse_identifier(WAVE_TYPE_VOID, false);
            break;
        }

        case WAVE_TOKEN_OP_SEMICOLON: {
            parser_advance();
            break;
        }

        default: {
            switch (token_get_wave_type(token)) {
                case WAVE_TYPE_U8:
                case WAVE_TYPE_U16:
                case WAVE_TYPE_U32:
                case WAVE_TYPE_U64:

                case WAVE_TYPE_I8:
                case WAVE_TYPE_I16:
                case WAVE_TYPE_I32:
                case WAVE_TYPE_I64:

                case WAVE_TYPE_F32:
                case WAVE_TYPE_F64: {
                    parse_variable_initializer_statement();
                    break;
                }

                case WAVE_TYPE_STR: {

                    break;
                }

                case WAVE_TYPE_ARR:
                case WAVE_TYPE_ENUM:
                case WAVE_TYPE_STRUCT:
                case WAVE_TYPE_FUNC: {
                    PARSER_RAISE_ERROR("parse_statement", "not implemented yet"); // TODO
                    return;
                }

                case WAVE_TYPE_VOID: {
                    PARSER_RAISE_ERROR("parse_statement", "cannot use void as a variable type");
                    return;
                }

                default: {
                    parse_expression_statement();
                    break;
                }
            }

            break;
        }
    }

    if (compiler_has_error()) {
        return;
    }
}

static void parse_block(void) {
    PARSER_EXPECT(WAVE_TOKEN_OP_CURLY_BRACKET_OPEN, "parse_block", "expected start of block statement, missing opening curly bracket ('{')");

    u32 block_start_line = parser.current_line;
    u32 block_start_row = parser.current_row;

    while (parser.current.token != WAVE_TOKEN_OP_CURLY_BRACKET_CLOSE && parser.current.token != WAVE_TOKEN_FILE_END) {
        parse_statement();
        if (compiler_has_error()) {
            PARSER_RAISE_ERROR_AT("parse_block", "error in block statement", block_start_line, block_start_row);
            return;
        }
    }

    PARSER_EXPECT(WAVE_TOKEN_OP_CURLY_BRACKET_CLOSE, "parse_block", "expected end of block statement, missing closing curly bracket ('}')");
}

// parse statement

static void parse_expression_statement(void) {
    parse_expression(WAVE_TYPE_VOID);
    PARSER_EXPECT(WAVE_TOKEN_OP_SEMICOLON, "parse_expression_statement", "expected semicolon (';')");
}

static void parse_variable_initializer_statement(void) {
    wave_type variable_type = token_get_wave_type(parser.current.token);
    DEBUG_ASSERT(variable_type != WAVE_TYPE_NONE && variable_type != WAVE_TYPE_VOID, "unexpected variable type");
    parser_advance();

    PARSER_EXPECT(WAVE_TOKEN_IDENTIFIER, "parse_variable_initializer_statement", "variable name expected");
    string_hash identifier = PARSER_GET_DATA(wave_identifier, parser.previous.data_index).hash;

    wave_local* local_variable = add_local(variable_type, identifier, false);

    // check if the variable is an assign expression or if it has an initializer

    if (parser_match(WAVE_TOKEN_OP_SEMICOLON)) {
        return;
    }

    bool assign = parser_match(WAVE_TOKEN_OP_ASSIGN);

    // parse initializer expression

    parse_expression(variable_type);

    local_variable->initialized = true;

    // emit variable setter

    switch (wave_type_get_size(variable_type)) {
        case (sizeof(u8)):  { emit_byte(OPCODE_STORE_8);  break; }
        case (sizeof(u16)): { emit_byte(OPCODE_STORE_16); break; }
        case (sizeof(u32)): { emit_byte(OPCODE_STORE_32); break; }
        case (sizeof(u64)): { emit_byte(OPCODE_STORE_64); break; }

        default: {
            PARSER_RAISE_ERROR("parse_variable_initializer_statement", "unknown variable type");
            return;
        }
    }

    emit_u16(local_variable->offset);

    // end of statement

    PARSER_EXPECT(WAVE_TOKEN_OP_SEMICOLON, "parse_variable_initializer_statement", "expected semicolon (';')");
}

static void parse_variable_string_initializer_statement(void) {
    PARSER_EXPECT(WAVE_TOKEN_KEYWORD_STR, "parse_variable_string_initializer_statement", "expected string type variable (\"%s\" keyword)", (str_format_data) keyword_tokens[WAVE_TOKEN_KEYWORD_STR].string);

    PARSER_EXPECT(WAVE_TOKEN_IDENTIFIER, "parse_variable_initializer_statement", "variable name expected");
    string_hash identifier = PARSER_GET_DATA(wave_identifier, parser.previous.data_index).hash;

    wave_local* local_variable = add_local(WAVE_TYPE_STR, identifier, false);

    // check if the variable is an assign expression or if it has an initializer

    if (parser_match(WAVE_TOKEN_OP_SEMICOLON)) {
        return;
    }

    // TODO: assign expression
}

static void parse_function_call_statement(bool reference_function_call, wave_type* out_return_type) {
    PARSER_EXPECT(WAVE_TOKEN_IDENTIFIER, "parse_function_call_statement", "expected function name");
    string_hash identifier_name = PARSER_GET_DATA(wave_identifier, parser.previous.data_index).hash;

    DEBUG_INFO("parse_function_call_statement: identifier_name = %x64", identifier_name);

    // resolve function

    bool native_function = false;
    u16 native_function_index = 0;
    parse_function function = PARSE_FUNCTION_NULL;

    if (reference_function_call) {
        wave_type type = WAVE_TYPE_NONE;
        if (!resolve_variable(identifier_name, &type)) {
            PARSER_RAISE_ERROR("parse_function_call_statement", "unknown variable");
            return;
        }

        if (type != WAVE_TYPE_FUNC) {
            PARSER_RAISE_ERROR("parse_function_call_statement", "variable wrong type, expected function type (\"%s\")", (str_format_data) keyword_tokens[WAVE_TOKEN_KEYWORD_FUNC].string);
            return;
        }
    } else {
        bool function_defined = false;
        for (u32 i = 0; i < parser.functions_count; i++) {
            if (parser.functions[i].function_data.name == identifier_name) {
                function = parser.functions[i];
                function_defined = true;
                break;
            }
        }

        if (!function_defined) {
            for (u32 i = 0; i < parser.vm->function_stack_length; i++) {
                if (parser.vm->native_functions[i].function_data.name == identifier_name) {
                    function.function_data = parser.vm->native_functions[i].function_data;
                    native_function_index = i;

                    function_defined = true;
                    break;
                }
            }

            if (!function_defined) {
                PARSER_RAISE_ERROR("parse_function_call_statement", "failed to resolve function, unknown function");
                return;
            }

            native_function = true;
        }
    }

    *out_return_type = function.function_data.return_type;

    // parse parameters

    PARSER_EXPECT(WAVE_TOKEN_OP_PARENTHESES_OPEN, "parse_function_call_statement", "expected parameter list, missing opening parentheses ('(')");

    for (u32 i = 0; i < function.function_data.parameter_count; i++) {
        wave_type parameter_type = function.function_data.parameters[i].type;

        if (parser.current.token == WAVE_TOKEN_OP_PARENTHESES_CLOSE && function.function_data.parameters[i].has_default_value) {
            union_number default_value = function.function_data.parameters[i].default_value;

            switch (parameter_type) {
                case WAVE_TYPE_U8:
                case WAVE_TYPE_I8: { emit_u8(default_value.value_u8); break; }

                case WAVE_TYPE_U16:
                case WAVE_TYPE_I16: { emit_u16(default_value.value_u16); break; }

                case WAVE_TYPE_U32:
                case WAVE_TYPE_I32:
                case WAVE_TYPE_F32: { emit_u32(default_value.value_u32); break; }

                case WAVE_TYPE_U64:
                case WAVE_TYPE_I64:
                case WAVE_TYPE_F64: { emit_u64(default_value.value_u64); break; }

                case WAVE_TYPE_FUNC:

                case WAVE_TYPE_STR:
                case WAVE_TYPE_ARR:

                case WAVE_TYPE_ENUM:
                case WAVE_TYPE_STRUCT: {
                    PARSER_RAISE_ERROR("parse_function_call_statement", "currently there is no support for default values for heap objects");
                    return;
                }

                default: {
                    PARSER_RAISE_ERROR("parse_function_call_statement", "unknown variable type");
                    return;
                }
            }

            continue;
        }

        if (parser.current.token == WAVE_TOKEN_OP_PARENTHESES_CLOSE) {
            PARSER_RAISE_ERROR("parse_function_call_statement", "incomplete function call, expected %u parameters, got %u", function.function_data.parameter_count, i);
            return;
        } else if (parser.current.token == WAVE_TOKEN_FILE_END) {
            PARSER_RAISE_ERROR("parse_function_call_statement", "incomplete function call, unexpected end of file, missing closing parentheses (')')");
            return;
        }

        // parse parameter expression

        parse_expression(parameter_type);

        // continue to next parameter

        if (i == function.function_data.parameter_count - 1) {
            break;
        } else {
            PARSER_EXPECT(WAVE_TOKEN_OP_COMMA, "parse_function_call_statement", "expected parameter separation, missing comma (',') between parameters");
        }
    }

    PARSER_EXPECT(WAVE_TOKEN_OP_PARENTHESES_CLOSE, "parse_function_call_statement", "too many parameters for function call, expected %u parameters", function.function_data.parameter_count);

    // emit function call

    if (!function.initialized) {
        // TODO: add the current bytecode offset to a list of "patch holes"
    }

    if (reference_function_call) {
        emit_variable(identifier_name, true, false);

        if (native_function) {
            if (function.function_data.error_function) {
                emit_byte(OPCODE_CALL_DYN_ERR);
            } else {
                emit_byte(OPCODE_CALL_DYN);
            }
        } else {
            if (function.function_data.error_function) {
                emit_byte(OPCODE_CALL_DYN_ERR);
                emit_byte(OPCODE_ERR_CHECK);
            } else {
                emit_byte(OPCODE_CALL_DYN);
            }
        }
    } else {
        if (native_function) {
            if (function.function_data.error_function) {
                emit_byte(OPCODE_CALL_NATIVE_ERR);
                emit_u16(native_function_index);
            } else {
                emit_byte(OPCODE_CALL_NATIVE);
                emit_u16(native_function_index);
            }
        } else {
            if (function.function_data.error_function) {
                emit_byte(OPCODE_CALL);
                emit_u16(function.branch_offset);
                emit_byte(OPCODE_ERR_CHECK);
            } else {
                emit_byte(OPCODE_CALL);
                emit_u32(function.branch_offset);
            }
        }
    }
}

static void parse_if_statement(void) {}
static void parse_do_statement(void) {}
static void parse_while_statement(void) {}
static void parse_for_statement(void) {}
static void parse_switch_statement(void) {}

static void parse_return_statement(void) {
    DEBUG_ASSERT(function_parser.scope_depth >= 1, "not inside a function scope");

    if ((*parser.current_function).function_data.return_type == WAVE_TYPE_NONE) {
        goto parse_return_statement_end;
    }

    // TODO: pop stack

    parse_expression((*(parser.current_function)).function_data.return_type);

    parse_return_statement_end: {}

    PARSER_EXPECT(WAVE_TOKEN_OP_SEMICOLON, "parse_return_statement", "expected semicolon (';') at the end of a statement");

    emit_byte(OPCODE_RETURN);
}

static void parse_exit_statement(void) {
    DEBUG_ASSERT(function_parser.scope_depth >= 1, "not inside a function scope");

    if ((*parser.current_function).function_data.return_type == WAVE_TYPE_NONE) {
        goto parse_exit_statement_end;
    }

    parse_expression(WAVE_TYPE_U16);

    parse_exit_statement_end: {}

    PARSER_EXPECT(WAVE_TOKEN_OP_SEMICOLON, "parse_exit_statement", "expected semicolon (';') at the end of a statement");

    emit_byte(OPCODE_END);
}

static void parse_label_statement(void) {
    PARSER_EXPECT(WAVE_TOKEN_IDENTIFIER, "parse_label_statement", "missing label name, expected identifier token");
    wave_identifier identifier = PARSER_GET_DATA(wave_identifier, parser.current.data_index);
    parse_label label = (parse_label) {
        .name = identifier.hash,
        .branch_offset = parser.bytecode_current - parser.bytecode_start
    };

    // TODO: push to label list

    PARSER_EXPECT(WAVE_TOKEN_OP_COLON, "parse_label_statement", "expected colon (':') after label statement");
}

// parse declaration

static void parse_function_parameters(parse_parameter** out_parameters, bool* out_function_forward_declared) {
    function_parser.locals_offset = 0;

    parse_function* function = parser.current_function;
    wave_function* function_data = &parser.current_function->function_data;

    bool function_forward_declared = false;

    // pare parameter start

    if (!parser_consume(WAVE_TOKEN_OP_PARENTHESES_OPEN)) {
        PARSER_RAISE_ERROR("parse_function_parameters", "expected parameter list; missing opening parentheses ('(')");
        *out_parameters = NULL;
        *out_function_forward_declared = false;
        return;
    }

    parse_parameter* parameters = NULL;
    u16 parameter_capacity = 32;
    function_data->parameter_count = 0;
    if (parser.vm->allocate_memory((void**) &parameters, sizeof(parse_parameter) * parameter_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        PARSER_RAISE_ERROR("parse_function_parameters", "failed to allocate function parameter stack");
        *out_parameters = NULL;
        *out_function_forward_declared = false;
        return;
    }

    while (!parser_match(WAVE_TOKEN_OP_PARENTHESES_CLOSE)) { // TODO: parse ( <type> <name>, <type> <name> = <expression> )
        wave_type parameter_type = WAVE_TYPE_NONE;
        string_hash parameter_name = 0;

        union_number default_value = (union_number) { .value_u64 = 0 };

        // parse type

        parameter_type = token_get_wave_type(parser.current.token);
        if (parameter_type == WAVE_TYPE_VOID) {
            PARSER_RAISE_ERROR("parse_function_parameters", "type void (\"%s\") cannot be a parameter variable type", (str_format_data) keyword_tokens[WAVE_TOKEN_KEYWORD_VOID].string);
            *out_parameters = parameters;
            *out_function_forward_declared = false;
            return;
        } else if (parameter_type == WAVE_TYPE_NONE) {
            PARSER_RAISE_ERROR("parse_function_parameters", "unknown parameter variable type");
            *out_parameters = parameters;
            *out_function_forward_declared = false;
            return;
        }

        parser_advance();

        // parse name

        if (parser_match(WAVE_TOKEN_IDENTIFIER)) {
            parameter_name = PARSER_GET_DATA(wave_identifier, parser.previous.data_index).hash;

            if (parser_match(WAVE_TOKEN_OP_ASSIGN)) {
                if (function_forward_declared) {
                    parser_reverse();
                    PARSER_RAISE_WARNING("parse_function_parameters", "default values for pre-declared functions are ignored");
                    *out_parameters = parameters;
                    *out_function_forward_declared = false;
                    return;
                }

                // TODO: parse default parameters (<type> <identifier> = <arithmetic_expression>)
            }
        } else {
            function_forward_declared = true;
        }

        // push parameter to function parameter stack

        parse_parameter parameter = (parse_parameter) {
            .name = parameter_name,
            .type = parameter_type,
            .offset = function_parser.locals_offset,

            .default_value = default_value
        };

        STACK_HELPER_PUSH(
            parameters,
            parameter,

            sizeof(parse_parameter),

            parameter_capacity,
            function_data->parameter_count,

            8,

            "parse_function_parameters",
            "failed to reallocate function parameter stack"
        );

        parser_match(WAVE_TOKEN_OP_COMMA);
    }

    // allocate function data parameters

    if (function_data->parameter_count > 0) {
        if (parser.vm->allocate_memory((void**) &function_data->parameters, sizeof(wave_parameter) * function_data->parameter_count) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            PARSER_RAISE_ERROR("parse_function_parameters", "failed to allocate function parameter stack");
            *out_parameters = parameters;
            *out_function_forward_declared = false;
            return;
        }

        for (u16 i = 0; i < function_data->parameter_count; i++) {
            function_data->parameters[i] = (wave_parameter) {
                .type = parameters[i].type,
                .has_default_value = false,
                .default_value = 0
            };
        }
    }

    // parse return type

    function_data->return_type = WAVE_TYPE_NONE;

    u32 return_type_start_line = parser.current_line;
    u32 return_type_start_row = parser.current_row;

    if (parser_match(WAVE_TOKEN_OP_COLON)) {
        function_data->return_type = token_get_wave_type(parser.current.token);
        if (function_data->return_type == WAVE_TYPE_NONE) {
            PARSER_RAISE_ERROR_AT("parse_function_parameters", "unknown function return type, expected type token", return_type_start_line, return_type_start_row);
            *out_parameters = parameters;
            *out_function_forward_declared = false;
            return;
        }

        parser_advance();
    } else {
        function_data->return_type = WAVE_TYPE_VOID;
    }

    function_parser.locals_offset = 0;

    *out_parameters = parameters;
    *out_function_forward_declared = false;
}

static void parse_function_body(str function_name_source_pointer, u32 function_start_line, u32 function_start_row, const parse_parameter* parameters) {
    function_parser.locals_offset = 0;

    parse_function* function = parser.current_function;
    wave_function* function_data = &parser.current_function->function_data;

    // emit debug information for disassembler (in front of root sets)

    emit_byte(OPCODE_DEBUG);
    emit_byte(DEBUG_INSTRUCTION_TYPE_FUNCTION_START);

    emit_u64(function->function_data.name); // function name

    u32* debug_root_set_size = (u32*) parser.bytecode_current; emit_u32(0); // root_set_size

    if (function_name_source_pointer != NULL) {
        u32* function_name_length = (u32*) parser.bytecode_current; emit_u32(0);

        u32 length = 0;
        while (wave_compiler_builtin_char_is_namespace(*function_name_source_pointer)) {
            emit_byte(*((byte *) function_name_source_pointer));
            function_name_source_pointer++;
            length++;
        }

        *function_name_length = length;
    } else {
        emit_u32(0);
    }

    // make space for @globals_root_set_size @locals_root_set_size and @locals_stack_frame_size

    u16* globals_root_set_size   = (u16*) parser.bytecode_current; emit_u16(0);
    u16* locals_root_set_size    = (u16*) parser.bytecode_current; emit_u16(0);

    function->branch_offset = parser.bytecode_current - parser.bytecode_start; // @OPCODE_CALL expects to be passed the offset to @parameter_size (16bit), followed by @locals_stack_frame_size (16bit)

    u16* parameter_size          = (u16*) parser.bytecode_current; emit_u16(0);
    u16* locals_stack_frame_size = (u16*) parser.bytecode_current; emit_u16(0);

    // add parameters as locals

    for (u16 i = 0; i < function_data->parameter_count; i++) {
        add_local(parameters[i].type, parameters[i].name, true);

        *parameter_size += wave_type_get_size(parameters[i].type);
    }

    // parse function body

    PARSER_EXPECT(WAVE_TOKEN_OP_CURLY_BRACKET_OPEN, "parse_function_body", "expected start of function body, missing opening curly bracket ('{')");

    WAVE_COMPILER_DEBUG("parse_function_body: parse function body start");

    parser_begin_scope();
    while (parser.current.token != WAVE_TOKEN_OP_CURLY_BRACKET_CLOSE && parser.current.token != WAVE_TOKEN_FILE_END) {
        parse_statement();
        if (compiler_has_error()) {
            PARSER_RAISE_ERROR_AT("parse_function_body", "error in function ...", function_start_line, function_start_row); // TODO: print function name
            return;
        }
    }

    PARSER_EXPECT(WAVE_TOKEN_OP_CURLY_BRACKET_CLOSE, "parse_function_body", "expected end of function body, missing closing curly bracket ('}')");
    parser_end_scope();

    WAVE_COMPILER_DEBUG("parse_function_body: parse function body end");

    // end function

    function->locals_size = function_parser.locals_offset - *parameter_size;

    *locals_stack_frame_size = function->locals_size;
    DEBUG_INFO("locals_stack_frame_size: %u", function->locals_size);

    // construct root sets

    WAVE_COMPILER_DEBUG("parse_function_body: construct root set");

    // TODO

    // end function

    emit_byte(OPCODE_DEBUG);
    emit_byte(DEBUG_INSTRUCTION_TYPE_FUNCTION_END);

    WAVE_COMPILER_DEBUG("parse_function_body: close");
}

static void parse_function_declaration(void) {
    parse_function function = PARSE_FUNCTION_NULL;

    // initialize state variables

    function_parser.locals_count = 0;
    function_parser.locals_offset = 0;

    function_parser.scope_depth = 0;

    parser.current_function = &function;

    // parse function modifiers

    function.inline_function = false;
    function.function_data.error_function = false;

    bool extern_function = false;
    bool event_function = false; // TODO: implement

    bool asm_function = false; // TODO: implement

    while (!parser_match(WAVE_TOKEN_KEYWORD_FUNC)) {
        switch (parser.current.token) {
            case WAVE_TOKEN_KEYWORD_INLINE: { function.inline_function = true; parser_advance(); break; }
            case WAVE_TOKEN_KEYWORD_EXTERN: { extern_function          = true; parser_advance(); break; }
            case WAVE_TOKEN_KEYWORD_EVENT:  { event_function           = true; parser_advance(); break; }
            case WAVE_TOKEN_KEYWORD_ERROR:  { function.function_data.error_function = true; parser_advance(); break; }
            case WAVE_TOKEN_KEYWORD_ASM:    { asm_function             = true; parser_advance(); break; }

            default: {
                PARSER_RAISE_ERROR("parse_function_declaration", "unknown function modifier");
                return;
            }
        }

        parser_advance();
    }

    // parse function name

    PARSER_EXPECT(WAVE_TOKEN_IDENTIFIER, "parse_function_declaration", "missing function name, expected identifier token");
    wave_identifier identifier = PARSER_GET_DATA(wave_identifier, parser.previous.data_index);
    function.function_data.name = identifier.hash;

    u32 function_start_line = parser.previous_line;
    u32 function_start_row = parser.previous_row;

    // parse function parameters

    parse_parameter* parameters = NULL;
    bool function_forward_declared = false;
    parse_function_parameters(&parameters, &function_forward_declared);
    if (parameters == NULL) {
        PARSER_RAISE_ERROR("parse_function_declaration", "failed to parse function parameters");
        return;
    }

    // check if the function body is defined

    if (!function_forward_declared || parser_match(WAVE_TOKEN_OP_SEMICOLON)) {
        function.initialized = false;
    } else {
        for (u32 i = 0; i < parser.functions_count; i++) {
            if (parser.functions[i].initialized && parser.functions[i].function_data.name == function.function_data.name) {
                PARSER_RAISE_ERROR_AT("parse_function_declaration", "function already defined", function_start_line, function_start_row); // TODO: print function name
                return;
            }
        }
    }

    // parse function body

    parse_function_body(identifier.source_pointer, function_start_line, function_start_row, parameters); // TODO: add recursion support

    if (parser.vm->deallocate_memory(parameters) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        PARSER_RAISE_ERROR("parse_function_declaration", "failed to deallocate function parameter stack");
        return;
    }

    function.initialized = true;

    // store function data

    STACK_HELPER_PUSH(
        parser.functions,
        function,

        sizeof(wave_function),

        parser.functions_capacity,
        parser.functions_count,

        FUNCTION_STACK_GROW_SIZE,

        "parse_function_declaration",
        "failed to reallocate function stack"
    );

    // push extern function to stack if modifier is set

    if (extern_function || event_function) {
        STACK_HELPER_PUSH(
            parser.extern_functions,
            parser.functions_count,

            sizeof(u32),

            parser.extern_functions_capacity,
            parser.extern_functions_count,

            EXTERN_FUNCTION_STACK_GROW_SIZE,

            "parse_function_declaration",
            "failed to reallocate extern function stack"
        );
    }

    // reset state variables

    function_parser.locals_count = 0;
    function_parser.locals_offset = 0;

    function_parser.scope_depth = 0;

    parser.current_function = NULL;
}

static void parse_entrypoint_declaration(void) {
    if (parser.entrypoint_function.initialized) {
        PARSER_RAISE_ERROR("parse_entrypoint_declaration", "redefinition of entrypoint function");
        return;
    }

    // initialize state variables

    function_parser.locals_count = 0;
    function_parser.locals_offset = 0;

    function_parser.scope_depth = 0;

    parse_function function = PARSE_FUNCTION_NULL;
    parser.current_function = &function;

    function.branch_offset = 0;

    // parse entrypoint and parameters

    u32 function_start_line = parser.current_line;
    u32 function_start_row = parser.current_row;

    PARSER_EXPECT(WAVE_TOKEN_KEYWORD_ENTRYPOINT, "parse_entrypoint_declaration", "expected entrypoint keyword (\"%s\")", (str_format_data) keyword_tokens[WAVE_TOKEN_KEYWORD_ENTRYPOINT].string);

    function.function_data.name = 0;

    // parse function parameters

    parser.current_scope_is_entrypoint_function = true;

    parse_parameter* parameters = NULL;
    bool function_forward_declared = false;
    parse_function_parameters(&parameters, &function_forward_declared);
    if (parameters == NULL) {
        PARSER_RAISE_ERROR("parse_entrypoint_declaration", "failed to parse function parameters");
        return;
    }

    // parse function body

    if (!parser_match(WAVE_TOKEN_OP_SEMICOLON)) {
        parse_function_body(NULL, function_start_line, function_start_row, parameters);
    }

    if (parser.vm->deallocate_memory(parameters) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
        PARSER_RAISE_ERROR("parse_entrypoint_declaration", "failed to deallocate function parameter stack");
        return;
    }

    parser.current_scope_is_entrypoint_function = false;
    function.initialized = true;

    // store entrypoint function

    parser.entrypoint_function = *parser.current_function;

    // reset state variables

    function_parser.locals_count = 0;
    function_parser.locals_offset = 0;

    function_parser.scope_depth = 0;

    parser.current_function = NULL;
}

static void parse_global_variable_declaration(void) {
    wave_type variable_type = token_get_wave_type(parser.current.token);
    DEBUG_ASSERT(variable_type != WAVE_TYPE_NONE && variable_type != WAVE_TYPE_VOID, "unexpected variable type");
    parser_advance();

    if (parser_match(WAVE_TOKEN_OP_ASSIGN)) {
        parse_expression(variable_type);

        u16 prev_globals_offset = parser.globals_offset;
        switch (wave_type_get_size(variable_type)) {
            case (sizeof(u8)):  { emit_byte(OPCODE_SET_GLOB_8);  parser.globals_offset += sizeof(u8);  break; }
            case (sizeof(u16)): { emit_byte(OPCODE_SET_GLOB_16); parser.globals_offset += sizeof(u16); break; }
            case (sizeof(u32)): { emit_byte(OPCODE_SET_GLOB_32); parser.globals_offset += sizeof(u32); break; }
            case (sizeof(u64)): { emit_byte(OPCODE_SET_GLOB_64); parser.globals_offset += sizeof(u64); break; }

            default: {
                PARSER_RAISE_ERROR("parse_global_variable_declaration", "unknown variable type");
                return;
            }
        }

        emit_u16(prev_globals_offset);
    }

    PARSER_EXPECT(WAVE_TOKEN_OP_SEMICOLON, "parse_global_variable_declaration", "expected semicolon (';')");
}

static void parse_local_variable_declaration(void) {
    wave_type variable_type = token_get_wave_type(parser.current.token);
    if (variable_type == WAVE_TYPE_VOID) {
        PARSER_RAISE_ERROR("parse_local_variable_declaration", "unexpected; local with type void (\"%s\")", (str_format_data) keyword_tokens[WAVE_TOKEN_KEYWORD_VOID].string);
        return;
    } else if (variable_type == WAVE_TYPE_NONE) {
        PARSER_RAISE_ERROR("parse_local_variable_declaration", "unknown variable type");
        return;
    }

    parser_advance();

    string_hash variable_name = PARSER_GET_DATA(wave_identifier, parser.current.data_index).hash;
    for (i32 i = ((i32) function_parser.locals_count) - 1; i >= 0; i--) {
        wave_local* local = &function_parser.locals[i];
        if (local->depth != -1 && local->depth < function_parser.scope_depth) {
            break;
        }

        if (variable_name == local->name) {
            PARSER_RAISE_ERROR("parse_local_variable_declaration", "local variable already defined");
            return;
        }
    }

    if (parser.current.token == WAVE_TOKEN_OP_SQUARE_BRACKET_OPEN) {
        variable_type = WAVE_TYPE_ARR; // TODO: this if statement only covers the cases 1. <type>[] <name>; and 2. <type>[<number>] <name>; but not 3. <type>[<expression>] <name>;
    }

    add_local(variable_type, variable_name, false);
}

static void parse_enum_declaration(void) {}
static void parse_struct_declaration(void) {}
static void parse_union_declaration(void) {}

static void parse_precedence(wave_type expression_type, parsing_precedence precedence) {
    DEBUG_ASSERT(precedence != PRECEDENCE_NONE, "precedence should not be PRECEDENCE_NONE when parsing expressions, may have left the bounds of the expression or met an unexpected token");

    parser_advance();

    bool can_assign = precedence <= PRECEDENCE_ASSIGNMENT;

    parse_rule *prefix_rule = parser_get_rule(parser.previous.token);
    if (prefix_rule->prefix != PARSE_RULE_FUNC_NONE) {
        parsing_function prefix_function = parse_rule_function_table[prefix_rule->prefix];
        if (prefix_rule == NULL) {
            PARSER_RAISE_ERROR("parse_precedence", "expected expression");
            return;
        }

        prefix_function(expression_type, can_assign);
    }

    while (precedence <= parser_get_rule(parser.current.token)->precedence) {
        DEBUG_INFO("prec: %u --- line: %u, row: %u", parser_get_rule(parser.current.token)->precedence, parser.current_line, parser.current_row);
        parser_advance();
        parse_rule_function_table[parser_get_rule(parser.previous.token)->infix](expression_type, can_assign);
    }

    if (can_assign && parser_match(WAVE_TOKEN_OP_ASSIGN)) {
        PARSER_RAISE_ERROR("parse_precedence", "invalid assignment target");
        return;
    }

    return;
}

// variables

static wave_local* add_local(wave_type type, string_hash name, bool initialized) {
    DEBUG_ASSERT(type != WAVE_TYPE_NONE, "unexpected variable type");

    umax type_size = wave_type_get_size(type);

    if ((function_parser.locals_offset + type_size) >= WAVE_LIMIT_MAX_LOCALS_OFFSET) {
        PARSER_RAISE_ERROR("add_local", "too many local variables defined");
        return NULL;
    } else if (function_parser.locals_count + 1 >= function_parser.locals_capacity) {
        function_parser.locals_capacity += LOCALS_STACK_GROW_SIZE;
        if (parser.vm->reallocate_memory((void**) &(function_parser.locals), sizeof(wave_local) * function_parser.locals_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            PARSER_RAISE_ERROR("add_local", "failed to reallocate function local variable stack");
            return NULL;
        }
    }

    wave_local* local = &function_parser.locals[function_parser.locals_count];
    function_parser.locals_count++;

    local->name = name;
    local->type = type;

    local->initialized = initialized;

    local->offset = function_parser.locals_offset;
    local->depth = function_parser.scope_depth;

    function_parser.locals_offset += type_size;

    return local;
}

static u32 resolve_local(string_hash name, wave_local* out_variable) {
    for (u32 i = 0; i < function_parser.locals_count; i++) {
        if (function_parser.locals[i].name == name) {
            if (!function_parser.locals[i].initialized) {
                PARSER_RAISE_ERROR("resolve_local", "cannot read variable in its own initializer");
                goto resolve_local_end;
            }

            *out_variable = function_parser.locals[i];
            return true;
        }
    }

    resolve_local_end: {}

    *out_variable = (wave_local) { .name = 0, .type = WAVE_TYPE_NONE, .offset = 0, .depth = 0 };
    return false;
}

static void add_global(wave_type type, string_hash name) {
    DEBUG_ASSERT(type != WAVE_TYPE_NONE, "unexpected variable type");

    umax type_size = wave_type_get_size(type);

    if ((parser.globals_offset + type_size) > WAVE_LIMIT_MAX_GLOBALS_OFFSET) {
        PARSER_RAISE_ERROR("add_global", "too many global variables defined");
        return;
    } else if (parser.globals_count >= parser.globals_capacity) {
        parser.globals_capacity += GLOBALS_STACK_GROW_SIZE;
        if (parser.vm->reallocate_memory((void**) &(parser.globals), sizeof(wave_global) * parser.globals_capacity) != ERROR_CODE_EXECUTION_SUCCESSFUL) {
            PARSER_RAISE_ERROR("add_global", "failed to reallocate global variable stack");
            return;
        }
    }

    wave_global* global = &parser.globals[parser.globals_count];
    parser.globals_count++;

    global->name = name;
    global->offset = parser.globals_offset;
    global->type = type;

    parser.globals_offset += type_size;
}

static bool resolve_global(string_hash name, wave_global* out_variable) {
    for (u32 i = 0; i < parser.globals_count; i++) {
        if (parser.globals[i].name == name) {
            if (!parser.globals[i].initialized) {
                PARSER_RAISE_ERROR("resolve_global", "cannot read variable in its own initializer");
                goto resolve_global_end;
            }

            *out_variable = parser.globals[i];
            return true;
        }
    }

    resolve_global_end: {}

    *out_variable = (wave_global) { .name = 0, .type = WAVE_TYPE_NONE, .offset = 0, .initialized = false };
    return false;
}

static bool resolve_variable(string_hash name, wave_type* out_type) {
    wave_local local_variable;
    wave_global global_variable;

    if (resolve_local(name, &local_variable)) {
        *out_type = local_variable.type;
        return true;
    } else if (resolve_global(name, &global_variable)) {
        *out_type = global_variable.type;
        return true;
    } else {
        *out_type = WAVE_TYPE_NONE;
        return false;
    }
}

static bool emit_variable(string_hash name, bool evaluate, bool assign_expression) {
    byte get_operation;
    byte set_operation;

    wave_local local_variable;
    wave_global global_variable;

    u16 offset = 0;

    if (resolve_local(name, &local_variable)) {
        switch (wave_type_get_size(local_variable.type)) {
            case (sizeof(u8)):  { get_operation = OPCODE_LOAD_8;  set_operation = OPCODE_STORE_8;  break; }
            case (sizeof(u16)): { get_operation = OPCODE_LOAD_16; set_operation = OPCODE_STORE_16; break; }
            case (sizeof(u32)): { get_operation = OPCODE_LOAD_32; set_operation = OPCODE_STORE_32; break; }
            case (sizeof(u64)): { get_operation = OPCODE_LOAD_64; set_operation = OPCODE_STORE_64; break; }

            default: {
                PARSER_RAISE_ERROR("emit_variable", "unknown variable type");
                return false;
            }
        }

        offset = local_variable.offset;
    } else if (resolve_global(name, &global_variable)) {
        switch (wave_type_get_size(global_variable.type)) {
            case (sizeof(u8)):  { get_operation = OPCODE_GET_GLOB_8;  set_operation = OPCODE_SET_GLOB_8;  break; }
            case (sizeof(u16)): { get_operation = OPCODE_GET_GLOB_16; set_operation = OPCODE_SET_GLOB_16; break; }
            case (sizeof(u32)): { get_operation = OPCODE_GET_GLOB_32; set_operation = OPCODE_SET_GLOB_32; break; }
            case (sizeof(u64)): { get_operation = OPCODE_GET_GLOB_64; set_operation = OPCODE_SET_GLOB_64; break; }

            default: {
                PARSER_RAISE_ERROR("emit_variable", "unknown variable type");
                return false;
            }
        }

        offset = global_variable.offset;
    } else {
        return false;
    }

    if (assign_expression) {
        parse_expression(WAVE_TYPE_VOID);
        if (evaluate) {
            emit_byte(set_operation);
            emit_u16(offset);
        }
    } else if (evaluate) {
        emit_byte(get_operation);
        emit_u16(offset);
    }

    return true;
}

// functions

static bool function_is_defined(string_hash name) {
    for (u32 i = 0; i < parser.functions_count; i++) {
        if (parser.functions[i].function_data.name == name) {
            return true;
        }
    }
    for (u32 i = 0; i < parser.vm->function_stack_length; i++) {
        if (parser.vm->native_functions[i].function_data.name == name) {
            return true;
        }
    }

    return false;
}

static bool resolve_function(string_hash name, parse_function* out_function) {
    for (u32 i = 0; i < parser.functions_count; i++) {
        if (parser.functions[i].function_data.name == name) {
            *out_function = parser.functions[i];
            return true;
        }
    }

    *out_function = PARSE_FUNCTION_NULL;
    return false;
}

// Parse Rule Functions

static void parse_literal(wave_type expression_type, bool can_assign) {
    parse_token* token = &parser.previous;

    // TODO: literal should be handled similar to number, meaning a lookahead is required, maybe resolve literals at tokenization stage

    // cast the literal to the desired type

    switch (expression_type) {
        case WAVE_TYPE_U8:
        case WAVE_TYPE_I8: {
            emit_byte(OPCODE_PUSH_8);
            switch (token->token) {
                case WAVE_TOKEN_KEYWORD_VALUE_TRUE:  { emit_u8(1); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_FALSE: { emit_u8(0); break; }

                default: { goto parser_literal_error_case; }
            }

            break;
        }

        case WAVE_TYPE_U16:
        case WAVE_TYPE_I16: {
            emit_byte(OPCODE_PUSH_16);
            switch (token->token) {
                case WAVE_TOKEN_KEYWORD_VALUE_TRUE:  { emit_u16(1); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_FALSE: { emit_u16(0); break; }

                default: { goto parser_literal_error_case; }
            }

            break;
        }

        case WAVE_TYPE_U32:
        case WAVE_TYPE_I32:
        case WAVE_TYPE_F32: {
            emit_byte(OPCODE_PUSH_32);
            switch (token->token) {
                case WAVE_TOKEN_KEYWORD_VALUE_TRUE:  { emit_u32(1); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_FALSE: { emit_u32(0); break; }

                case WAVE_TOKEN_KEYWORD_VALUE_NAN: { f32 temp = F32_NAN; emit_u32(*((u32*) &temp)); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_INF: { f32 temp = F32_INF; emit_u32(*((u32*) &temp)); break; }

                default: { goto parser_literal_error_case; }
            }

            break;
        }

        case WAVE_TYPE_U64:
        case WAVE_TYPE_I64:
        case WAVE_TYPE_F64: {
            emit_byte(OPCODE_PUSH_64);
            switch (token->token) {
                case WAVE_TOKEN_KEYWORD_VALUE_TRUE:  { emit_u64(1); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_FALSE: { emit_u64(0); break; }

                case WAVE_TOKEN_KEYWORD_VALUE_NAN: { f64 temp = F64_NAN; emit_u64(*((u64*) &temp)); break; }
                case WAVE_TOKEN_KEYWORD_VALUE_INF: { f64 temp = F64_INF; emit_u64(*((u64*) &temp)); break; }

                default: { goto parser_literal_error_case; }
            }

            break;
        }

        case WAVE_TYPE_STR:
        case WAVE_TYPE_ARR:

        case WAVE_TYPE_ENUM:
        case WAVE_TYPE_STRUCT:

        case WAVE_TYPE_FUNC: {
            if (token->token == WAVE_TOKEN_KEYWORD_VALUE_NULL) {
                emit_byte(OPCODE_PUSH_64);
                emit_u64(0);
                break;
            } else {
                goto parser_literal_error_case;
            }
        }

        parser_literal_error_case:
        default: {
            PARSER_RAISE_ERROR("parse_literal", "unknown variable type");
            return;
        }
    }
}

static void parse_number(wave_type expression_type, bool can_assign) {
    parse_token number = parser.previous; // TODO: this needs to know it's required type, based on @left_expression_type and @right_expression_type

    // implicit cast to current expression type

    switch (number.token) {
        case WAVE_TOKEN_VALUE_INTEGER: {
            u64 integer = PARSER_GET_DATA(u64, number.data_index);

            /* TODO: use this in ast generation?
            wave_type integer_type = WAVE_TYPE_NONE; // choose the lowest possible type
            if (integer <= U8_MAX) {
                integer_type = WAVE_TYPE_U8;
            } else if (integer <= U16_MAX) {
                integer_type = WAVE_TYPE_U16;
            } else if (integer <= U32_MAX) {
                integer_type = WAVE_TYPE_U32;
            } else {
                integer_type = WAVE_TYPE_U64;
            }
            */

            switch (expression_type) {
                case WAVE_TYPE_U8:  { emit_byte(OPCODE_PUSH_8);  emit_u8((u8) integer);   break; }
                case WAVE_TYPE_U16: { emit_byte(OPCODE_PUSH_16); emit_u16((u16) integer); break; }
                case WAVE_TYPE_U32: { emit_byte(OPCODE_PUSH_32); emit_u32((u32) integer); break; }
                case WAVE_TYPE_U64: { emit_byte(OPCODE_PUSH_64); emit_u64((u64) integer); break; }

                case WAVE_TYPE_I8:  { emit_byte(OPCODE_PUSH_8);  i8 temp  = (i8) integer;  emit_u8(*((u8*) (&temp)));   break; }
                case WAVE_TYPE_I16: { emit_byte(OPCODE_PUSH_16); i16 temp = (i16) integer; emit_u16(*((u16*) (&temp))); break; }
                case WAVE_TYPE_I32: { emit_byte(OPCODE_PUSH_32); i32 temp = (i32) integer; emit_u32(*((u32*) (&temp))); break; }
                case WAVE_TYPE_I64: { emit_byte(OPCODE_PUSH_64); i64 temp = (i64) integer; emit_u64(*((u64*) (&temp))); break; }

                case WAVE_TYPE_F32: { emit_byte(OPCODE_PUSH_32); f32 temp = (f32) integer; emit_u32(*((u32*) (&temp))); break; }
                case WAVE_TYPE_F64: { emit_byte(OPCODE_PUSH_64); f64 temp = (f64) integer; emit_u64(*((u64*) (&temp))); break; }

                default: {
                    PARSER_RAISE_ERROR("parse_number", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        case WAVE_TOKEN_VALUE_FLOAT: {
            wave_float float_value = PARSER_GET_DATA(wave_float, number.data_index);

            f32 f32_value = float_value.f32;
            f64 f64_value = float_value.f64;

            wave_type float_type = WAVE_TYPE_NONE;
            if (f32_value <= F32_MAX && f64_value <= (f64) F32_MAX) {
                float_type = WAVE_TYPE_F32;
            } else {
                float_type = WAVE_TYPE_F64;
            }

            // for floats the tokenizer stores both float sizes and the better fitting one is chosen

            switch (expression_type) {
                case WAVE_TYPE_U8:  { emit_byte(OPCODE_PUSH_8);  emit_u8((u8) f32_value);   break; }
                case WAVE_TYPE_U16: { emit_byte(OPCODE_PUSH_16); emit_u16((u16) f32_value); break; }
                case WAVE_TYPE_U32: { emit_byte(OPCODE_PUSH_32); emit_u32((u32) f32_value); break; }
                case WAVE_TYPE_U64: { emit_byte(OPCODE_PUSH_64); emit_u64((u64) f64_value); break; }

                case WAVE_TYPE_I8:  { emit_byte(OPCODE_PUSH_8);  i8 temp  = (i8) f32_value;  emit_u8(*((u8*) (&temp)));   break; }
                case WAVE_TYPE_I16: { emit_byte(OPCODE_PUSH_16); i16 temp = (i16) f32_value; emit_u16(*((u16*) (&temp))); break; }
                case WAVE_TYPE_I32: { emit_byte(OPCODE_PUSH_32); i32 temp = (i32) f32_value; emit_u32(*((u32*) (&temp))); break; }
                case WAVE_TYPE_I64: { emit_byte(OPCODE_PUSH_64); i64 temp = (i64) f64_value; emit_u64(*((u64*) (&temp))); break; }

                case WAVE_TYPE_F32: { emit_byte(OPCODE_PUSH_32); emit_u32(*((u32*) (&f32_value))); break; }
                case WAVE_TYPE_F64: { emit_byte(OPCODE_PUSH_64); emit_u64(*((u64*) (&f64_value))); break; }

                default: {
                    PARSER_RAISE_ERROR("parse_number", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        default: {
            PARSER_RAISE_ERROR("parse_number", "unable to parse number, unexpected token");
            return;
        }
    }
}

static void parse_string(wave_type expression_type, bool can_assign) {
    parser_advance();

    parse_token token = parser.previous;
    u32 string_length = PARSER_GET_DATA(u32, token.data_index);
    str string_start = PARSER_GET_DATA(str, token.data_index + sizeof(u32));

    emit_byte(OPCODE_STR_NEW);
    emit_u32(string_length);
    for (u32 i = 0; i < string_length; i++) {
        emit_byte(*((byte*) string_start));
        string_start++;
    }
}

static void parse_identifier(wave_type expression_type, bool can_assign) {
    string_hash name = PARSER_GET_DATA(wave_identifier, parser.previous.data_index).hash;

    // resolve the variable (local or global)

    wave_type type = WAVE_TYPE_NONE;
    if (resolve_variable(name, &type)) {
        DEBUG_ASSERT(type != WAVE_TYPE_NONE, "unknown variable type");

        // check if the variable is an assign expression

        bool assign = parser_match(WAVE_TOKEN_OP_ASSIGN) && can_assign;

        emit_variable(name, true, assign);

        // if the variable is used in an expression cast it to the desired type

        if (!assign && type != expression_type) { // TODO: lookahead the desired type (maybe generate an ast and type check before code generation)
            number_type convert_from = (number_type) type;
            number_type convert_to = (number_type) expression_type;

            emit_byte(OPCODE_TYPE_CONV_STATIC);
            emit_byte((convert_from << 4) | (convert_to << 0));
        }
    } else if (function_is_defined(name)) {
        parser_reverse();
        wave_type function_return_type = WAVE_TYPE_NONE;
        parse_function_call_statement(false, &function_return_type);
    } else {
        PARSER_RAISE_ERROR_PREV("parse_identifier", "unknown identifier");
        return;
    }
}

static void parse_dot(wave_type expression_type, bool can_assign) {
    // TODO
}

static void parse_unary(wave_type expression_type, bool can_assign) {
    wave_token operator_token = parser.previous.token;

    // parse the expression first

    parse_precedence(expression_type, PRECEDENCE_ASSIGNMENT);

    // then emit the unary opcode

    switch (operator_token) {
        case WAVE_TOKEN_OP_NEG: {
            switch (expression_type) {
                case WAVE_TYPE_U8:
                case WAVE_TYPE_I8: { emit_byte(OPCODE_I8_NEG); }

                case WAVE_TYPE_U16:
                case WAVE_TYPE_I16: { emit_byte(OPCODE_I16_NEG); }

                case WAVE_TYPE_U32:
                case WAVE_TYPE_I32: { emit_byte(OPCODE_I32_NEG); }

                case WAVE_TYPE_U64:
                case WAVE_TYPE_I64: { emit_byte(OPCODE_I64_NEG); }

                case WAVE_TYPE_F32: { emit_byte(OPCODE_F32_NEG); }
                case WAVE_TYPE_F64: { emit_byte(OPCODE_F64_NEG); }

                default: {
                    PARSER_RAISE_ERROR("parse_unary", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        case WAVE_TOKEN_OP_ABS: {
            switch (expression_type) {
                case WAVE_TYPE_I8: { emit_byte(OPCODE_I8_ABS); }
                case WAVE_TYPE_I16: { emit_byte(OPCODE_I16_ABS); }
                case WAVE_TYPE_I32: { emit_byte(OPCODE_I32_ABS); }
                case WAVE_TYPE_I64: { emit_byte(OPCODE_I64_ABS); }

                case WAVE_TYPE_F32: { emit_byte(OPCODE_F32_ABS); }
                case WAVE_TYPE_F64: { emit_byte(OPCODE_F64_ABS); }

                case WAVE_TYPE_U8:
                case WAVE_TYPE_U16:
                case WAVE_TYPE_U32:
                case WAVE_TYPE_U64: {
                    break;
                }

                default: {
                    PARSER_RAISE_ERROR("parse_unary", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        case WAVE_TOKEN_OP_BIT_NOT: {
            switch (expression_type) {
                case WAVE_TYPE_U8: case WAVE_TYPE_I8: { emit_byte(OPCODE_BNOT_8); }
                case WAVE_TYPE_U16: case WAVE_TYPE_I16: { emit_byte(OPCODE_BNOT_16); }
                case WAVE_TYPE_U32: case WAVE_TYPE_I32: case WAVE_TYPE_F32: { emit_byte(OPCODE_BNOT_32); }
                case WAVE_TYPE_U64: case WAVE_TYPE_I64: case WAVE_TYPE_F64: { emit_byte(OPCODE_BNOT_64); }

                default: {
                    PARSER_RAISE_ERROR("parse_unary", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        case WAVE_TOKEN_OP_NOT: {
            switch (expression_type) {
                case WAVE_TYPE_U8: case WAVE_TYPE_I8: { emit_byte(OPCODE_NOT_8); }
                case WAVE_TYPE_U16: case WAVE_TYPE_I16: { emit_byte(OPCODE_NOT_16); }
                case WAVE_TYPE_U32: case WAVE_TYPE_I32: case WAVE_TYPE_F32: { emit_byte(OPCODE_NOT_32); }
                case WAVE_TYPE_U64: case WAVE_TYPE_I64: case WAVE_TYPE_F64: { emit_byte(OPCODE_NOT_64); }

                default: {
                    PARSER_RAISE_ERROR("parse_unary", "invalid expression variable type");
                    return;
                }
            }

            break;
        }

        default: {
            PARSER_RAISE_ERROR("parse_unary", "unable to parse unary expression, unexpected operator token");
            return;
        }
    }
}

static void parse_binary(wave_type expression_type, bool can_assign) {
    wave_token operator_token = parser.previous.token;
    parse_rule* rule = parser_get_rule(operator_token);

    // obtain the left and right expression parts type

    parse_precedence(expression_type, (parsing_precedence) (rule->precedence + 1));
    wave_type result_expression_type = wave_type_get_higher(expression_type, expression_type);

    // macros

    #define DEFAULT_CASE() default: { PARSER_RAISE_ERROR("parse_binary", "invalid expression variable type"); return; }
    #define TYPE_CASE(operation, type) case CONCAT2(WAVE_TYPE_, type): { emit_byte(CONCAT4(OPCODE_, type, _, operation)); break; }
    #define TYPE_CASES(operation)   \
        TYPE_CASE(operation, U8)    \
        TYPE_CASE(operation, U16)   \
        TYPE_CASE(operation, U32)   \
        TYPE_CASE(operation, U64)   \
                                    \
        TYPE_CASE(operation, I8)    \
        TYPE_CASE(operation, I16)   \
        TYPE_CASE(operation, I32)   \
        TYPE_CASE(operation, I64)   \
                                    \
        TYPE_CASE(operation, F32)   \
        TYPE_CASE(operation, F64)   \

    #define CASE_ARITHMETIC_OPCODE(opcode, operation)   \
        case opcode: {                                  \
            switch (result_expression_type) {           \
                TYPE_CASES(operation)                   \
                DEFAULT_CASE()                          \
            }                                           \
                                                        \
            break;                                      \
        }

    #define CASE_BITWISE_OPCODE(opcode, operation)                                                                                  \
        case opcode: {                                                                                                              \
            switch (result_expression_type) {                                                                                       \
                case WAVE_TYPE_U8:  case WAVE_TYPE_I8:  { emit_byte(CONCAT3(OPCODE_, operation, _8));  break; }                       \
                case WAVE_TYPE_U16: case WAVE_TYPE_I16: { emit_byte(CONCAT3(OPCODE_, operation, _16)); break; }                       \
                case WAVE_TYPE_U32: case WAVE_TYPE_I32: case WAVE_TYPE_F32: { emit_byte(CONCAT3(OPCODE_, operation, _32)); break; }    \
                case WAVE_TYPE_U64: case WAVE_TYPE_I64: case WAVE_TYPE_F64: { emit_byte(CONCAT3(OPCODE_, operation, _64)); break; }    \
                                                                                                                                    \
                DEFAULT_CASE()                                                                                                      \
            }                                                                                                                       \
                                                                                                                                    \
            break;                                                                                                                  \
        }

    // then emit the binary expression opcode

    switch (operator_token) { // TODO: add mid-expression assign support (TOKEN_OP_ASSIGN_ADD, ..ASSIGN_SUB, ...);
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_POS, ADD)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_NEG, SUB)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_MUL, MUL)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_DIV, DIV)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_MOD, MOD)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_POW, POW)

        case WAVE_TOKEN_OP_EQUAL: {
            switch (result_expression_type) {
                case WAVE_TYPE_U8:  case WAVE_TYPE_I8:  { emit_byte(OPCODE_EQU_8);  break; }
                case WAVE_TYPE_U16: case WAVE_TYPE_I16: { emit_byte(OPCODE_EQU_16); break; }
                case WAVE_TYPE_U32: case WAVE_TYPE_I32: { emit_byte(OPCODE_EQU_32); break; }
                case WAVE_TYPE_U64: case WAVE_TYPE_I64: { emit_byte(OPCODE_EQU_64); break; }

                case WAVE_TYPE_F32: {
                    emit_byte(OPCODE_F32_EQU);
                    break;
                }

                case WAVE_TYPE_F64: {
                    emit_byte(OPCODE_F64_EQU);
                    break;
                }

                DEFAULT_CASE()
            }

            break;
        }

        case WAVE_TOKEN_OP_UNEQUAL: {
            switch (result_expression_type) {
                case WAVE_TYPE_U8:  case WAVE_TYPE_I8:  { emit_byte(OPCODE_NEQ_8);  break; }
                case WAVE_TYPE_U16: case WAVE_TYPE_I16: { emit_byte(OPCODE_NEQ_16); break; }
                case WAVE_TYPE_U32: case WAVE_TYPE_I32: { emit_byte(OPCODE_NEQ_32); break; }
                case WAVE_TYPE_U64: case WAVE_TYPE_I64: { emit_byte(OPCODE_NEQ_64); break; }

                case WAVE_TYPE_F32: {
                    emit_byte(OPCODE_F32_NEQ);
                    break;
                }

                case WAVE_TYPE_F64: {
                    emit_byte(OPCODE_F64_NEQ);
                    break;
                }

                DEFAULT_CASE()
            }

            break;
        }

        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_AND, AND)
        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_OR, OR)

        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_BIT_AND, BAND)
        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_BIT_OR, BOR)
        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_BIT_XOR, XOR)
        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_BIT_SHIFT_LEFT, SHIFT_L)
        CASE_BITWISE_OPCODE(WAVE_TOKEN_OP_BIT_SHIFT_RIGHT, SHIFT_R)

        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_LESS_THAN, LT)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_LESS_THAN_EQUAL, LE)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_GREATER_THAN, GT)
        CASE_ARITHMETIC_OPCODE(WAVE_TOKEN_OP_GREATER_THAN_EQUAL, GE)

        default: {
            PARSER_RAISE_ERROR("parse_binary", "unable to parse binary expression, unexpected operator token");
            return;
        }
    }

    #undef DEFAULT_CASE
    #undef TYPE_CASE
    #undef TYPE_CASES
    #undef CASE_ARITHMETIC_OPCODE
    #undef CASE_BITWISE_OPCODE
}

static void parse_grouping(wave_type expression_type, bool can_assign) {
    parse_precedence(expression_type, PRECEDENCE_ASSIGNMENT);
    PARSER_EXPECT(WAVE_TOKEN_OP_PARENTHESES_CLOSE, "parse_grouping", "expected end of grouping statement, missing closing parentheses (')')");
}

static void parse_type_conversion(wave_type expression_type, bool can_assign) {
    wave_type conversation_type = token_get_wave_type(parser.previous.token);

    // consume the first parentheses

    PARSER_EXPECT( WAVE_TOKEN_OP_PARENTHESES_OPEN, "parse_type_conversion", "expected cast parentheses, missing opening parentheses ('(')");

    // now that the top level type of the grouping expression is known, evaluate and emit the grouping expression

    parse_grouping(conversation_type, can_assign);

    // type convert the result

    // TODO: resolve types after ast generation

    /*
    number_type convert_from = (number_type) grouping_type;
    number_type convert_to = (number_type) conversation_type;

    emit_byte(OPCODE_TYPE_CONV_STATIC);
    emit_byte((convert_from << 4) | (convert_to << 0));
    */
}

// other

static bool is_function_modifier(wave_token token) {
    return token == WAVE_TOKEN_KEYWORD_INLINE || token == WAVE_TOKEN_KEYWORD_EXTERN || token == WAVE_TOKEN_KEYWORD_EVENT || token == WAVE_TOKEN_KEYWORD_ERROR;
}

// Exposed Functions

error_code wave_compiler_parser_compile(wave_vm* vm, parse_token* tokenized_start, parse_token* tokenized_end, byte* data_stack_start, byte* data_stack_end) {
    const wave_memory_allocation_function allocate_memory = vm->allocate_memory;
    const wave_memory_allocation_zero_function allocate_zero_memory = vm->allocate_zero_memory;
    const wave_memory_reallocation_function reallocate_memory = vm->reallocate_memory;
    const wave_memory_deallocation_function deallocate_memory = vm->deallocate_memory;

    // initialize parser

    parser.vm = vm;

    parser.tokenized_start   = tokenized_start;
    parser.tokenized_end     = tokenized_end;
    parser.tokenized_current = parser.tokenized_start;

    parser.data_stack_start = data_stack_start;
    parser.data_stack_end   = data_stack_end;

    parser.current_file_name = "undefined";
    parser.current_line = 0;
    parser.current_row = 0;

    parser.previous_line = 0;
    parser.previous_row = 0;

    parser.bytecode_start = NULL;
    parser.bytecode_capacity = BYTECODE_STACK_GROW_SIZE;
    RUN_ERROR_CODE_FUNCTION(allocate_zero_memory, (void**) &parser.bytecode_start, sizeof(byte) * parser.bytecode_capacity);
    vm->bytecode_start = parser.bytecode_start;
    parser.bytecode_end = parser.bytecode_start + parser.bytecode_capacity;
    parser.bytecode_current = parser.bytecode_start;

    parser.current  = (parse_token) { .token = WAVE_TOKEN_INVALID, .data_index = 0, .line = 0, .row = 0 };
    parser.previous = (parse_token) { .token = WAVE_TOKEN_INVALID, .data_index = 0, .line = 0, .row = 0 };

    parser.globals = NULL;
    parser.globals_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &parser.globals, sizeof(wave_global) * parser.globals_capacity);
    parser.globals_count = 0;
    parser.globals_offset = 0;

    parser.entrypoint_function = PARSE_FUNCTION_NULL;
    parser.current_scope_is_entrypoint_function = false;
    parser.current_function = NULL;
    parser.functions = NULL;
    parser.functions_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &parser.functions, sizeof(wave_function) * parser.functions_capacity);
    parser.functions_count = 0;

    parser.extern_functions = NULL;
    parser.extern_functions_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &parser.extern_functions, sizeof(u32) * parser.extern_functions_capacity);
    parser.extern_functions_count = 0;

    parser.patch_holes = NULL;
    parser.patch_hole_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &parser.patch_holes, sizeof(patch_hole) * parser.patch_hole_capacity);
    parser.patch_hole_count = 0;

    // initialize function parser

    function_parser.scope_depth = 0;

    function_parser.accessed_globals = NULL;

    function_parser.accessed_globals = NULL;
    function_parser.accessed_globals_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &function_parser.accessed_globals, sizeof(u16) * function_parser.accessed_globals_capacity);
    function_parser.accessed_globals_count = 0;

    function_parser.locals = NULL;
    function_parser.locals_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &function_parser.locals, sizeof(wave_local) * function_parser.locals_capacity);
    function_parser.locals_count = 0;

    function_parser.labels = NULL;
    function_parser.label_capacity = 32;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &function_parser.labels, sizeof(parse_label) * function_parser.label_capacity);
    function_parser.label_count = 0;

    WAVE_COMPILER_DEBUG("wave_compiler_parser_compile: everything allocated");

    // macros

    #define EMIT(function, value) do { function(value); if (compiler_has_error()) { vm->bytecode_start = parser.bytecode_start; vm->bytecode_end = parser.bytecode_current - 1; return ERROR_CODE_EXECUTION_FAILED; } } while (0)

    // function hash & function index

    EMIT(emit_u64, (u64) vm->function_hash);

    u32 bytecode_entrypoint_offset = parser.bytecode_current - parser.bytecode_start;
    EMIT(emit_u32, 0); // reserve space for the entrypoint branch offset
    EMIT(emit_u32, 0); // reserve space for the exposed function index length

    // run parser

    WAVE_COMPILER_DEBUG("wave_compiler_parser_compile: starting parser");

    parser_advance();
    while (!parser_match(WAVE_TOKEN_FILE_END)) {
        parse_declaration();
        if (compiler_has_error()) {
            break;
        }
    }

    WAVE_COMPILER_DEBUG("wave_compiler_parser_compile: parser finished");

    // end of parser

    if (!compiler_has_error() && *(parser.bytecode_current - 1) != OPCODE_END) {
        EMIT(emit_byte, OPCODE_END);
    }

    #undef EMIT

    // add entrypoint function

    if (parser.entrypoint_function.initialized) {
        *((u32*) (parser.bytecode_start + bytecode_entrypoint_offset)) = parser.entrypoint_function.branch_offset;
    }

    // fix patch holes

    // resize bytecode

    vm->bytecode_start = parser.bytecode_start;
    vm->bytecode_end = parser.bytecode_current - 1;
    RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(vm->bytecode_start), vm->bytecode_end - vm->bytecode_start);

    // return error, if any

    if (compiler_has_error()) {
        return ERROR_CODE_EXECUTION_FAILED;
    }

    // deallocate temporary memory

    wave_compiler_parser_destroy();

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_compiler_parser_destroy(void) {
    const wave_memory_allocation_function allocate_memory = parser.vm->allocate_memory;
    const wave_memory_deallocation_function deallocate_memory = parser.vm->deallocate_memory;

    #define PARSER_DEALLOCATE(pointer) do { if (pointer != NULL) { RUN_ERROR_CODE_FUNCTION(deallocate_memory, (void*) pointer); pointer = NULL; } } while (0)

    // deallocate temporary memory

    PARSER_DEALLOCATE(parser.globals);

    if (parser.functions != NULL) {
        for (u32 i = 0; i < parser.functions_count; i++) {
            if (parser.functions[i].function_data.parameters != NULL && parser.functions[i].function_data.parameter_count > 0) {
                RUN_ERROR_CODE_FUNCTION(deallocate_memory, parser.functions[i].function_data.parameters);
            }
        }

        PARSER_DEALLOCATE(parser.functions);
    }

    PARSER_DEALLOCATE(parser.extern_functions);

    PARSER_DEALLOCATE(parser.patch_holes);

    PARSER_DEALLOCATE(function_parser.accessed_globals);
    PARSER_DEALLOCATE(function_parser.locals);
    PARSER_DEALLOCATE(function_parser.labels);

    #undef PARSER_DEALLOCATE

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

// Macros

#undef PARSER_GET_DATA

#undef BYTECODE_FITS_SIZE
#undef BYTECODE_PUSH_DATA_UNSAFE
