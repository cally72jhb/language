#ifndef WAVE_LANGUAGE_PRECEDENCE
#define WAVE_LANGUAGE_PRECEDENCE

// Includes

#include "language/compiler/wave_compiler_common.h"
#include "language/compiler/wave_type.h"

// Typedefs

typedef enum {
    PRECEDENCE_NONE,

    PRECEDENCE_ASSIGNMENT,  // =  // += // -= // *= // /= // %= // **= // <<= // >>= // &= // |= // ^=
    PRECEDENCE_TERNARY,     // ?? // !!
    PRECEDENCE_OR,          // ||
    PRECEDENCE_AND,         // &&
    PRECEDENCE_BIT_OR,      // |
    PRECEDENCE_BIT_XOR,     // ^
    PRECEDENCE_BIT_AND,     // &
    PRECEDENCE_EQUALITY,    // == // !=
    PRECEDENCE_COMPARISON,  // <  // >  // <= // >=
    PRECEDENCE_SHIFT,       // << // >>
    PRECEDENCE_TERM,        // +  // -
    PRECEDENCE_FACTOR,      // *  // /  // %
    PRECEDENCE_POWER,       // **
    PRECEDENCE_UNARY,       // +/- (prefix)   // !  // :: // (<type>) (cast) // ++/-- (prefix)
    PRECEDENCE_PRIMARY      // .  // () // ++/-- (suffix)
} PRECEDENCES;
typedef byte parsing_precedence; // @PRECEDENCES

typedef enum {
    PARSE_RULE_FUNC_LITERAL = 0,
    PARSE_RULE_FUNC_NUMBER,
    PARSE_RULE_FUNC_STRING,

    PARSE_RULE_FUNC_IDENTIFIER,

    PARSE_RULE_FUNC_DOT,

    PARSE_RULE_FUNC_UNARY,
    PARSE_RULE_FUNC_BINARY,
    PARSE_RULE_FUNC_GROUPING,

    PARSE_RULE_FUNC_CAST,

    PARSE_RULE_FUNC_NONE,

    PARSE_RULE_FUNC_MAX
} PARSE_RULE_FUNCTIONS;
typedef byte parse_rule_function; // PARSE_RULE_FUNCTIONS

typedef void (*parsing_function) (wave_type expression_type, bool can_assign);
typedef struct {
    parse_rule_function prefix;
    parse_rule_function infix;
    parsing_precedence precedence;
} parse_rule;

// Precedence Functions

parse_rule* parser_get_rule(wave_token token);

#endif
