#ifndef TOKEN_ENTRY
#define TOKEN_ENTRY(...)
#endif

#ifndef KEYWORD_TOKENS
#define KEYWORD_TOKENS (0)
#endif

#ifndef VALUE_TOKENS
#define VALUE_TOKENS (0)
#endif

#ifndef OPERATOR_TOKENS
#define OPERATOR_TOKENS (0)
#endif

/* NOTE:
* Tokens are partially sorted in a way, that the string matching algorithm can pick the first match,
* meaning "abc" must not come before "abcdef" because then in some cases only "abc" is being matched.
*
* Some tokens don't have a constant string representation.
* */

////////////////////////////////////////////////////////////////
// Keyword Tokens                                             //
////////////////////////////////////////////////////////////////

#define USAGE_TYPE "<type> <identifier>; or: <type> <identifier> = <expression>;"

#if KEYWORD_TOKENS != 0

// Primitive Types

TOKEN_ENTRY(KEYWORD_U8,  "u8", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_U16, "u16", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_U32, "u32", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_U64, "u64", USAGE_TYPE)

TOKEN_ENTRY(KEYWORD_I8,  "i8", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_I16, "i16", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_I32, "i32", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_I64, "i64", USAGE_TYPE)

TOKEN_ENTRY(KEYWORD_F32, "f32", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_F64, "f64", USAGE_TYPE)

// Specific Primitive Types

TOKEN_ENTRY(KEYWORD_BOOL, "bool", USAGE_TYPE) // TODO: make these only predeclared typedefs that are included by default if the amount of keywords is then equal to 48
TOKEN_ENTRY(KEYWORD_CHAR, "char", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_UINT, "uint", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_INT,  "int", USAGE_TYPE)
TOKEN_ENTRY(KEYWORD_UMAX, "umax", USAGE_TYPE)

// Complex Types

TOKEN_ENTRY(KEYWORD_STRUCT, "struct", "struct <identifier> { <struct_field_list> }; or: struct <identifier> { <struct_field_list> }; (as raw type; used in typedef)")
TOKEN_ENTRY(KEYWORD_ENUM, "enum", "enum <identifier> { <enum_member_list> }; or: enum { <enum_member_list> }; (as raw type; used in typedef)") // TODO: enumerations should be stored in the smallest type possible

TOKEN_ENTRY(KEYWORD_STR, "str", "str <identifier>; or: str <identifier> = <string>; or: str <identifier> = <expression>;")

// Function Keywords

TOKEN_ENTRY(KEYWORD_ENTRYPOINT, "entrypoint", "entrypoint() { <statements> } or: entrypoint(u32 argument_count, str[] arguments);")

TOKEN_ENTRY(KEYWORD_FUNC, "func", "<modifiers> func <identifier>(<parameter_list>) : <return_type>; or: <modifiers> func <identifier>(<parameter_list>) : <return_type> { <statements> }")
TOKEN_ENTRY(KEYWORD_CALL, "call", "call <variable_identifier>(<parameter_list>); or: call <function_identifer>(<parameter_list>);") // TODO: used for when calling closures: call <name>(<parameters>); (to make the code more readable)
TOKEN_ENTRY(KEYWORD_RETURN, "return", "return; or: return <expression>;")
TOKEN_ENTRY(KEYWORD_VOID, "void", "function return type")

TOKEN_ENTRY(KEYWORD_INLINE, "inline", "function modifier")
TOKEN_ENTRY(KEYWORD_EXTERN, "extern", "function modifier")
TOKEN_ENTRY(KEYWORD_EVENT, "event",   "function modifier")
TOKEN_ENTRY(KEYWORD_ERROR, "error",   "function modifier")
TOKEN_ENTRY(KEYWORD_ASM, "asm",       "function modifier")

// Type Modifying Keywords

TOKEN_ENTRY(KEYWORD_CONST, "const",   "variable modifier")
TOKEN_ENTRY(KEYWORD_GLOBAL, "global", "variable modifier")

// Other Keywords

TOKEN_ENTRY(KEYWORD_EXIT, "exit", "exit; or: exit <expression>;")

TOKEN_ENTRY(KEYWORD_DELETE, "delete", "delete <variable_identifier>; (only for heap allocated data types: str, arr, struct)")

TOKEN_ENTRY(KEYWORD_SIZEOF, "sizeof", "sizeof(<type>); or: sizeof(<identifier>);")
TOKEN_ENTRY(KEYWORD_TYPEOF, "typeof", "typeof(<type>); or: typeof(<identifier>);")

// Control Flow Keywords

TOKEN_ENTRY(KEYWORD_LABEL, "label", "label <identifier>:")
TOKEN_ENTRY(KEYWORD_GOTO, "goto", "goto <label_identifier>;")
TOKEN_ENTRY(KEYWORD_IF, "if", "if (<expression>) { <statements> }")
TOKEN_ENTRY(KEYWORD_ELSE, "else", "if (<expression>) { <statements> } else { <statements> }")
TOKEN_ENTRY(KEYWORD_BREAK, "break", "break; or: break <label_identifier>;")
TOKEN_ENTRY(KEYWORD_DO, "do", "do { <statements> }")
TOKEN_ENTRY(KEYWORD_CATCH, "catch", "do { <statements> } catch (<error_code>) { <statements> }")
TOKEN_ENTRY(KEYWORD_WHILE, "while", "while (<expression>) { <statements> }; or: do { <statements> } while (<expression>); or: while ( <expression> );")
TOKEN_ENTRY(KEYWORD_FOR, "for", "for (<declaration>; <condition_expression>; <expression_list>) { <statements> } or: for (<declaration>; <condition_expression>; <expression_list>);")
TOKEN_ENTRY(KEYWORD_SWITCH, "switch", "switch (<expression>) { <switch_case_list> }")
TOKEN_ENTRY(KEYWORD_CASE, "case", "case <expression>: { <statements> }")
TOKEN_ENTRY(KEYWORD_DEFAULT, "default", "default: { <statements> }")

TOKEN_ENTRY(KEYWORD_TENARY_IF, "??", "<expression> ?? <condition_true> !! <condition_false>")
TOKEN_ENTRY(KEYWORD_TENARY_ELSE, "!!", "<expression> ?? <condition_true> !! <condition_false>")

// Keyword Values

TOKEN_ENTRY(KEYWORD_VALUE_TRUE, "true", "handled as values")
TOKEN_ENTRY(KEYWORD_VALUE_FALSE, "false", "handled as values")

TOKEN_ENTRY(KEYWORD_VALUE_NULL, "null", "handled as values")

TOKEN_ENTRY(KEYWORD_VALUE_NAN, "nan", "handled as values")
TOKEN_ENTRY(KEYWORD_VALUE_INF, "inf", "handled as values")

#endif

////////////////////////////////////////////////////////////////
// Value Tokens (only store the specific number value)        //
////////////////////////////////////////////////////////////////

#if VALUE_TOKENS != 0

TOKEN_ENTRY(VALUE_INTEGER, "", 0)
TOKEN_ENTRY(VALUE_FLOAT, "", 0)

TOKEN_ENTRY(VALUE_STR, "", 0)

TOKEN_ENTRY(IDENTIFIER, "", 0)

#endif

////////////////////////////////////////////////////////////////
// Operator Tokens                                            //
////////////////////////////////////////////////////////////////

#if OPERATOR_TOKENS != 0

// Simple Mathematical Operators

TOKEN_ENTRY(OP_INC, "++", USAGE_TYPE_PREFIX | USAGE_TYPE_POSTFIX) // x++; // ++x;
TOKEN_ENTRY(OP_DEC, "--", USAGE_TYPE_PREFIX | USAGE_TYPE_POSTFIX) // x--; // --x;

TOKEN_ENTRY(OP_ASSIGN_ADD, "+=",  USAGE_TYPE_ASSIGN) // x += y;
TOKEN_ENTRY(OP_ASSIGN_SUB, "-=",  USAGE_TYPE_ASSIGN) // x -= y;
TOKEN_ENTRY(OP_ASSIGN_MUL, "*=",  USAGE_TYPE_ASSIGN) // x *= y;
TOKEN_ENTRY(OP_ASSIGN_DIV, "/=",  USAGE_TYPE_ASSIGN) // x /= y;
TOKEN_ENTRY(OP_ASSIGN_MOD, "%=",  USAGE_TYPE_ASSIGN) // x %= y;
TOKEN_ENTRY(OP_ASSIGN_POW, "**=", USAGE_TYPE_ASSIGN) // x **= y;

TOKEN_ENTRY(OP_POS, "+",  USAGE_TYPE_BINARY | USAGE_TYPE_UNARY) // x + y; // +x;
TOKEN_ENTRY(OP_NEG, "-",  USAGE_TYPE_BINARY | USAGE_TYPE_UNARY) // x - y; // -x;
TOKEN_ENTRY(OP_DIV, "/",  USAGE_TYPE_BINARY) // x / y;
TOKEN_ENTRY(OP_MOD, "%",  USAGE_TYPE_BINARY) // x % y;
TOKEN_ENTRY(OP_POW, "**", USAGE_TYPE_BINARY) // x ** y;
TOKEN_ENTRY(OP_MUL, "*",  USAGE_TYPE_BINARY) // x * y;

TOKEN_ENTRY(OP_ABS, "::", USAGE_TYPE_UNARY) // ::x;

// Logical Operators

TOKEN_ENTRY(OP_EQUAL,   "==", USAGE_TYPE_BINARY) // x == y;
TOKEN_ENTRY(OP_UNEQUAL, "!=", USAGE_TYPE_BINARY) // x != y;
TOKEN_ENTRY(OP_AND, "&&", USAGE_TYPE_BINARY) // x && y;
TOKEN_ENTRY(OP_OR,  "||", USAGE_TYPE_BINARY) // x || y;

// Bitwise Operators

TOKEN_ENTRY(OP_ASSIGN_BIT_AND, "&=", USAGE_TYPE_ASSIGN) // x &= y;
TOKEN_ENTRY(OP_ASSIGN_BIT_OR,  "|=", USAGE_TYPE_ASSIGN) // x |= y;
TOKEN_ENTRY(OP_ASSIGN_BIT_XOR, "^=", USAGE_TYPE_ASSIGN) // x ^= y;
TOKEN_ENTRY(OP_ASSIGN_BIT_SHIFT_LEFT,  "<<=", USAGE_TYPE_ASSIGN) // x <<= y;
TOKEN_ENTRY(OP_ASSIGN_BIT_SHIFT_RIGHT, ">>=", USAGE_TYPE_ASSIGN) // x >>= y;

TOKEN_ENTRY(OP_BIT_AND, "&",          USAGE_TYPE_BINARY) // x & y;
TOKEN_ENTRY(OP_BIT_OR,  "|",          USAGE_TYPE_BINARY) // x | y;
TOKEN_ENTRY(OP_BIT_XOR, "^",          USAGE_TYPE_BINARY) // x ^ y;
TOKEN_ENTRY(OP_BIT_SHIFT_LEFT,  "<<", USAGE_TYPE_BINARY) // x << y;
TOKEN_ENTRY(OP_BIT_SHIFT_RIGHT, ">>", USAGE_TYPE_BINARY) // x >> y;

TOKEN_ENTRY(OP_BIT_NOT, "~", USAGE_TYPE_UNARY) // ~x;
TOKEN_ENTRY(OP_NOT, "!", USAGE_TYPE_UNARY) // !x;

// Comparison Operators

TOKEN_ENTRY(OP_LESS_THAN,          "<",  USAGE_TYPE_BINARY) // x < y;
TOKEN_ENTRY(OP_LESS_THAN_EQUAL,    "<=", USAGE_TYPE_BINARY) // x <= y;
TOKEN_ENTRY(OP_GREATER_THAN,       ">",  USAGE_TYPE_BINARY) // x > y;
TOKEN_ENTRY(OP_GREATER_THAN_EQUAL, ">=", USAGE_TYPE_BINARY) // x >= y;

// Parentheses

TOKEN_ENTRY(OP_PARENTHESES_OPEN,     "(", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_PARENTHESES_CLOSE,    ")", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_CURLY_BRACKET_OPEN,   "{", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_CURLY_BRACKET_CLOSE,  "}", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_SQUARE_BRACKET_OPEN,  "[", USAGE_TYPE_INDEX)
TOKEN_ENTRY(OP_SQUARE_BRACKET_CLOSE, "]", USAGE_TYPE_INDEX)

// Other

TOKEN_ENTRY(OP_ASSIGN, "=", USAGE_TYPE_ASSIGN)

TOKEN_ENTRY(OP_SEMICOLON, ";", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_COLON, ":", USAGE_TYPE_NONE)
TOKEN_ENTRY(OP_COMMA, ",", USAGE_TYPE_NONE)

TOKEN_ENTRY(OP_DOT, ".", USAGE_TYPE_POSTFIX)

#endif

// un-define macros

#undef KEYWORD_TOKENS
#undef VALUE_TOKENS
#undef OPERATOR_TOKENS

#undef TOKEN_ENTRY
