#include "wave_ast.h"

#include "common/constants.h"
#include "common/error_codes.h"

// Functions

error_code wave_ast_new(wave_ast* ast, umax initial_size, wave_memory_allocation_function allocate_memory, wave_memory_reallocation_function reallocate_memory, wave_memory_deallocation_function deallocate_memory) {
    ast->allocate_memory = allocate_memory;
    ast->reallocate_memory = reallocate_memory;
    ast->deallocate_memory = deallocate_memory;

    ast->nodes = NULL;
    RUN_ERROR_CODE_FUNCTION(allocate_memory, (void**) &(ast->nodes), sizeof(wave_ast_node) * initial_size);
    ast->node_capacity = initial_size;
    ast->node_count = 0;

    ast->start_node = WAVE_AST_NODE_END;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_ast_resize(wave_ast* ast, umax grow_size) {
    const wave_memory_reallocation_function reallocate_memory = ast->reallocate_memory;

    ast->node_capacity += grow_size;
    RUN_ERROR_CODE_FUNCTION(reallocate_memory, (void**) &(ast->nodes), sizeof(wave_ast_node) * ast->node_capacity);

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_ast_destroy(wave_ast* ast) {
    const wave_memory_deallocation_function deallocate_memory = ast->deallocate_memory;

    if (ast->nodes != NULL) {
        RUN_ERROR_CODE_FUNCTION(deallocate_memory, ast->nodes);
    }

    ast->node_capacity = 0;
    ast->node_count = 0;

    ast->start_node = WAVE_AST_NODE_END;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_ast_insert(wave_ast* ast, wave_ast_node node) {
    if (ast->node_count >= ast->node_capacity) {
        RUN_ERROR_CODE_FUNCTION(wave_ast_resize, ast, 512);
    }

    ast->nodes[ast->node_count] = node;
    ast->node_count++;

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
