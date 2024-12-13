#include "hash.h"

#include <rapidhash.h>

#include "common/constants.h"

// Functions

string_hash hash_bytes(byte* bytes, u32 length) {
    return (string_hash) rapidhash((void*) bytes, length);
}
