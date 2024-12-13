#ifndef STANDARD_LIBRARY_HASH
#define STANDARD_LIBRARY_HASH

// Includes

#include "common/constants.h"

// Typedefs

typedef u64 string_hash;

// Functions

string_hash hash_bytes(byte* bytes, u32 length);

#endif
