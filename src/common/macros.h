#ifndef STANDARD_LIBRARY_MACROS
#define STANDARD_LIBRARY_MACROS

// Includes

#include "common/constants.h"
#include "common/defines.h"

// Defines

// macro helpers

#define EVAL(...) __VA_ARGS__

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) a##b
#define CONCAT3(a, b, c) a##b##c
#define CONCAT4(a, b, c, d) a##b##c##d
#define CONCAT5(a, b, c, d, e) a##b##c##d##e
#define CONCAT6(a, b, c, d, e, f) a##b##c##d##e##f
#define CONCAT7(a, b, c, d, e, f, g) a##b##c##d##e##f##g
#define CONCAT8(a, b, c, d, e, f, g, h) a##b##c##d##e##f##g##h

#define CAT(a, b)        CONCAT(a, b)
#define CAT2(a, b)       CONCAT2(a, b)
#define CAT3(a, b, c)    CONCAT3(a, b, c)
#define CAT4(a, b, c, d) CONCAT4(a, b, c, d)

#define STRINGIFY(variable) #variable

// length macros

#define STRING_LENGTH(string) ((umax) (sizeof(string) / sizeof(char)))
#define ARRAY_LENGTH(array) ((umax) (sizeof(array) / sizeof(array[0])))

// assertation

#define COMPILE_ASSERT(predicate, message) typedef char CONCAT2(assertion_failed_, message)[2 * !!(predicate) - 1]

// other

#define EMPTY_CODE_BLOCK() do { } while (0)

#define COUNT_ARGS(...) (sizeof((umax[]) { __VA_ARGS__ }) / sizeof(umax))
#define STRUCT_OFFSET_OF(structure, member) ((umax) &(((structure*) 0)->member))

#endif
