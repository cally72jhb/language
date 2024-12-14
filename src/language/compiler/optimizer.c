#include "optimizer.h"

// Functions

static error_code wave_optimizer_constant_fold(const wave_ast* ast) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

static error_code wave_optimizer_operator_strength_reduction(const wave_ast* ast) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

static error_code wave_optimizer_eliminate_dead_branches(const wave_ast* ast) {
    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}

error_code wave_optimizer_optimize(const wave_ast* ast) {
    RUN_ERROR_CODE_FUNCTION(wave_optimizer_constant_fold, ast);
    RUN_ERROR_CODE_FUNCTION(wave_optimizer_operator_strength_reduction, ast);
    RUN_ERROR_CODE_FUNCTION(wave_optimizer_eliminate_dead_branches, ast);

    // TODO: add loop motion here? (move loop-irrelevant statements out of the loop)

    return ERROR_CODE_EXECUTION_SUCCESSFUL;
}
