#ifndef WAVE_LANGUAGE_OPTIMIZER
#define WAVE_LANGUAGE_OPTIMIZER

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

#include "language/compiler/data/wave_ast.h"

// Functions

error_code wave_optimizer_optimize(const wave_ast* ast);

#endif
