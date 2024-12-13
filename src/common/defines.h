#ifndef STANDARD_LIBRARY_DEFINES
#define STANDARD_LIBRARY_DEFINES

// Program Constants

#define PROGRAM_VERSION_PART_0 (2) /* x.0.0 */
#define PROGRAM_VERSION_PART_1 (1) /* 0.x.0 */
#define PROGRAM_VERSION_PART_2 (0) /* 0.0.x  */

#define PROGRAM_NAME "standard_library"
#define PROGRAM_IDENTIFIER "standard_library"
#define PROGRAM_ENGINE "standard_library"

#define PROGRAM_ROOT_PATH "../"

// Compile-Time Features

// Debugging

#define PROGRAM_FEATURE_DEBUG_MODE (1) /* enables debug mode for the program, this should not be enabled when releasing, as it drastically slows down code generation and overall program performance */
#define PROGRAM_FEATURE_EXTERN_CONSOLE (0) /* creates an extern console for debug information */

#define PROGRAM_FEATURE_STACK_TRACE (1)           /* debugs every function that failed executing */
#define PROGRAM_FEATURE_DEBUG_STACK_TRACE (1)     /* prints out the file and line information for every debug call */
#define PROGRAM_FEATURE_STACK_TRACE_FUNCTIONS (1) /* (required PROGRAM_FEATURE_STACK_TRACE) debugs every function call made */

#define PROGRAM_FEATURE_NO_ERROR_CODE_NAMES (0) /* if this is set to 0, a table is created that maps all error codes to their corresponding names, which can be used for debugging, but requires more space to do so */
#define PROGRAM_FEATURE_NO_CONSOLE_COLORS (0)   /* if this is set to 0 ansi colors are used to highlight certain elements in the debugging console */
#define PROGRAM_FEATURE_NO_COLOR_REGISTRY (0)   /* if this is set to 1 the color registry table is not generated, saving up program space */

#define PROGRAM_FEATURE_WAVE_COMPILER_DEBUG_MODE (1) /* debugs compilation steps taken, useful while working on the compiler */

// Safety Features

#define PROGRAM_FEATURE_SAFE_MEMORY (1) /* enables extra checks in the allocation and deallocation functions */
#define PROGRAM_FEATURE_SAFE_PATH (0) /* checks whether paths leave the root directory (PROGRAM_ROOT_PATH) and throws an error if they do */

// Program Features

#define PROGRAM_FEATURE_32BIT_MODE (0) /* disables certain 64bit features, like copying data in word sizes of 64bit instead of 32bit */

#define PROGRAM_FEATURE_NO_OPCODE_NAMES (0) /* if this is set to 0 a table containing the name of every opcode is generated which may use up more program space */

#endif
