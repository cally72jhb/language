#ifndef WAVE_LANGUAGE_WAVE_AST
#define WAVE_LANGUAGE_WAVE_AST

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/wave_common.h"

// Typedefs

typedef enum {
    NODE_TYPE_NONE,

    // structures

    NODE_TYPE_LIST, // a node containing multiple nodes in an array structure

    // functions

    NODE_TYPE_FUNCTION_DEFINITION, // contains one @NODE_TYPE_FUNCTION_PARAMETER_LIST node and one @NODE_TYPE_STATEMENT_BLOCK node
    NODE_TYPE_FUNCTION_PARAMETER_LIST, // contains function parameters

    // statements

    NODE_TYPE_STATEMENT_BLOCK, // a block statement node // TODO: merge with @NODE_TYPE_LIST ?
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_FOR,

    NODE_TYPE_STATEMENT_VARIABLE_DECLARATION,

    NODE_TYPE_STATEMENT_FUNCTION_CALL,

    // expressions

    NODE_TYPE_EXPRESSION, // an expression statement

    NODE_TYPE_OPERATION_START, // marks the start of the operation-node-types

    NODE_TYPE_UNARY_OPERATION_PLUS,
    NODE_TYPE_UNARY_OPERATION_MINUS,
    NODE_TYPE_UNARY_OPERATION_ABS,
    NODE_TYPE_UNARY_OPERATION_BIT_NOT,
    NODE_TYPE_UNARY_OPERATION_NOT,

    NODE_TYPE_BINARY_OPERATION_ADD,
    NODE_TYPE_BINARY_OPERATION_SUB,
    NODE_TYPE_BINARY_OPERATION_MUL,
    NODE_TYPE_BINARY_OPERATION_DIV,
    NODE_TYPE_BINARY_OPERATION_MOD,
    NODE_TYPE_BINARY_OPERATION_POW,

    NODE_TYPE_BINARY_OPERATION_EQUAL,
    NODE_TYPE_BINARY_OPERATION_UNEQUAL,
    NODE_TYPE_BINARY_OPERATION_AND,
    NODE_TYPE_BINARY_OPERATION_OR,

    NODE_TYPE_BINARY_OPERATION_LESS_THAN,
    NODE_TYPE_BINARY_OPERATION_LESS_THAN_EQUAL,
    NODE_TYPE_BINARY_OPERATION_GREATER_THAN,
    NODE_TYPE_BINARY_OPERATION_GREATER_THAN_EQUAL,

    NODE_TYPE_BINARY_OPERATION_BIT_AND,
    NODE_TYPE_BINARY_OPERATION_BIT_OR,
    NODE_TYPE_BINARY_OPERATION_BIT_XOR,
    NODE_TYPE_BINARY_OPERATION_BIT_SHIFT_LEFT,
    NODE_TYPE_BINARY_OPERATION_BIT_SHIFT_RIGHT,

    NODE_TYPE_OPERATION_END, // marks the end of the operation-node-types

    // other

    NODE_TYPE_END,

    NODE_TYPE_MAX
} NODE_TYPES;
typedef byte node_type; // @NODE_TYPES
COMPILE_ASSERT(NODE_TYPE_MAX <= 256, too_many_ast_node_types_defined);

typedef struct wave_ast_node {
    node_type type;

    union { // type specific node data
        struct { // @NODE_TYPE_LIST, @NODE_TYPE_STATEMENT_BLOCK
            struct wave_ast_node* next_node;
            umax node_count;
        } list_data;

        struct { // @NODE_TYPE_UNARY_OPERATION, @NODE_TYPE_BINARY_OPERATION; the operation type is stored in @type to save space
            struct wave_ast_node* left_node;
            struct wave_ast_node* right_node;
        } expression_data;

        struct { // @NODE_TYPE_FUNCTION_DEFINITION
            wave_type return_type : 4;

            bool has_function_body : 1;

            bool modifier_inline : 1;
            bool modifier_extern : 1;
            bool modifier_event  : 1;
            bool modifier_error  : 1;
            bool modifier_asm    : 1;
        } function_definition_data;

        struct { // @NODE_TYPE_STATEMENT_FUNCTION_CALL
            bool native_function  : 1;
            bool has_parameters   : 1;
            wave_type return_type : 4;

            umax parameter_count;
        } function_call_data;

        struct { // used for setting data to 0
            umax zero1;
            umax zero2;
        };
    };
} wave_ast_node;
COMPILE_ASSERT(WAVE_TYPE_VOID <= 15, too_many_variable_types_defined); // either update the bit field or remove a few types

typedef struct {
    wave_memory_allocation_function allocate_memory;
    wave_memory_reallocation_function reallocate_memory;
    wave_memory_deallocation_function deallocate_memory;

    wave_ast_node* nodes; // the linear memory that stores the nodes
    umax node_capacity;
    umax node_count;

    wave_ast_node start_node; // typically a @NODE_TYPE_LIST storing multiple functions
} wave_ast;

// Defines

#define WAVE_AST_NODE_END ((wave_ast_node) { .type = NODE_TYPE_END, .zero1 = 0, .zero2 = 0 })

// Functions

error_code wave_ast_new(wave_ast* ast, umax initial_size, wave_memory_allocation_function allocate_memory, wave_memory_reallocation_function reallocate_memory, wave_memory_deallocation_function deallocate_memory);
error_code wave_ast_resize(wave_ast* ast, umax grow_size);
error_code wave_ast_destroy(wave_ast* ast);

error_code wave_ast_insert(wave_ast* ast, wave_ast_node node);

#endif
