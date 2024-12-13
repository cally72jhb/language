#ifndef ERROR_CODE_ENTRY
#define ERROR_CODE_ENTRY(...)
#endif

////////////////////////////////////////////////////////////////
// Main Error Codes                                           //
////////////////////////////////////////////////////////////////

ERROR_CODE_ENTRY(EXECUTION_SUCCESSFUL,                                                          ERROR_FLAG_IGNORE) // the execution of the function was successful
ERROR_CODE_ENTRY(UNKNOWN_ERROR,                                                                 ERROR_FLAG_FATAL) // an unknown error occurred
ERROR_CODE_ENTRY(NOT_IMPLEMENTED,                                                               ERROR_FLAG_FATAL) // an unimplemented feature was executed

ERROR_CODE_ENTRY(EXECUTION_FAILED,                                                              ERROR_FLAG_FATAL) // the execution of the function failed, but all errors were caught and handled

////////////////////////////////////////////////////////////////
// General Unspecific Error Codes                             //
////////////////////////////////////////////////////////////////

ERROR_CODE_ENTRY(NULL_POINTER_DEREFERENCE,                                                      ERROR_FLAG_SEVERE)

////////////////////////////////////////////////////////////////
// Platform Function Error Codes                              //
////////////////////////////////////////////////////////////////

ERROR_CODE_ENTRY(FAILED_TO_CREATE_COMMANDLINE,                                                  ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(FAILED_TO_DESTROY_COMMANDLINE,                                                 ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(FAILED_TO_COMMANDLINE_PRINT,                                                   ERROR_FLAG_IGNORE)

ERROR_CODE_ENTRY(FAILED_TO_GET_TIME,                                                            ERROR_FLAG_SEVERE)

ERROR_CODE_ENTRY(FAILED_TO_RETRIEVE_MEMORY_SIZE,                                                ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(FAILED_TO_ALLOCATE,                                                            ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_RE_ALLOCATE,                                                         ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_FREE,                                                                ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_MEMORY_SET,                                                          ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_MEMORY_COPY,                                                         ERROR_FLAG_SEVERE)

ERROR_CODE_ENTRY(FAILED_TO_CREATE_FOLDER,                                                       ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_CREATE_FILE,                                                         ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_WRITE_FILE,                                                          ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(FAILED_TO_READ_FILE,                                                           ERROR_FLAG_SEVERE)

////////////////////////////////////////////////////////////////
// Platform Specific Error Codes                              //
////////////////////////////////////////////////////////////////

// Platform WIN64

ERROR_CODE_ENTRY(WIN64_FAILED_TO_RETRIEVE_HEAP_HANDLE,                                          ERROR_FLAG_SEVERE)
ERROR_CODE_ENTRY(WIN64_FAILED_CLOSE_HANDLE,                                                     ERROR_FLAG_SEVERE)

// Platform UNIX

//ERROR_CODE_ENTRY(UNIX_FAILED_CREATE_TEMP_FILE,                                                  ERROR_FLAG_SEVERE)
//ERROR_CODE_ENTRY(UNIX_FAILED_GET_DESCRIPTOR,                                                    ERROR_FLAG_SEVERE)
//ERROR_CODE_ENTRY(UNIX_PAGESIZE_ERROR,                                                           ERROR_FLAG_SEVERE)
//ERROR_CODE_ENTRY(UNIX_TRUNCATE_ERROR,                                                           ERROR_FLAG_SEVERE)
//ERROR_CODE_ENTRY(UNIX_FAILED_TO_MMAP,                                                           ERROR_FLAG_SEVERE)

////////////////////////////////////////////////////////////////
// Path Specific Error Codes                                  //
////////////////////////////////////////////////////////////////

ERROR_CODE_ENTRY(PATH_TOO_LONG,                                                                 ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(PATH_INVALID,                                                                  ERROR_FLAG_WARNING)

////////////////////////////////////////////////////////////////
// Language Error Codes                                       //
////////////////////////////////////////////////////////////////

// Common

ERROR_CODE_ENTRY(LANGUAGE_INVALID_INSTRUCTIONSET,                                               ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_TOO_MANY_NATIVE_FUNCTIONS_DEFINED,                                    ERROR_FLAG_WARNING)

// Compiler

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_SUDDEN_END_OF_FILE,                                          ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNKNOWN_CHARACTER,                                           ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNKNOWN_TOKEN,                                               ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNKNOWN_TYPE,                                                ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNKNOWN_IDENTIFIER,                                          ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNKNOWN_VARIABLE_TYPE,                                       ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_UNEXPECTED_TOKEN,                                            ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_SEMICOLON,                                          ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_VARIABLE_NAME,                                      ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_OPENING_PARENTHESES,                                ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_CLOSING_PARENTHESES,                                ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_OPENING_BLOCK_STATEMENT,                            ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_CLOSING_BLOCK_STATEMENT,                            ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_EXPRESSION,                                         ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_INVALID_ASSIGNMENT_TARGET,                                   ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_EXPECTED_FUNC_KEYWORD,                                       ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_CANNOT_READ_VARIABLE_IN_ITS_OWN_INITIALIZER,                 ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_TOO_MANY_LOCAL_VARIABLES_DEFINED,                            ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_LOCAL_VARIABLE_ALREADY_DEFINED,                              ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_COMPILER_TOO_MANY_GLOBAL_VARIABLES_DEFINED,                           ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_COMPILER_INVALID_STRING,                                              ERROR_FLAG_WARNING)

// Runtime Errors

ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_BYTECODE_MISSING_FUNCTION_HASH,                               ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_BYTECODE_FUNCTION_HASH_NOT_MATCHING,                          ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_ENCOUNTERED_NOP_INSTRUCTION,                                  ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_INVALID_OPCODE,                                               ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_STACK_OVERFLOW,                                               ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_OPERATION_LEFT_STACK,                                         ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_JUMPED_OUT_OF_BYTECODE,                                       ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_CALL_STACK_OVERFLOW,                                          ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_CALL_STACK_UNDERFLOW,                                         ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_NULL_POINTER_EXCEPTION,                                       ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_INDEX_OUT_OF_BOUNDS,                                          ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_GLOBALS_INDEX_OUT_OF_BOUNDS,                                  ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_INVALID_NATIVE_FUNCTION_CALL,                                 ERROR_FLAG_WARNING)

ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_INVALID_SWITCH_CASE_VALUE,                                    ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_COPY_OPERATION_INVALID_SIZE,                                  ERROR_FLAG_WARNING)
ERROR_CODE_ENTRY(LANGUAGE_RUNTIME_TYPE_CONVERSION_INVALID_ARGUMENTS,                            ERROR_FLAG_WARNING)

#undef ERROR_CODE_ENTRY
