#ifndef STANDARD_LIBRARY_MEMORY
#define STANDARD_LIBRARY_MEMORY

// Includes

#include "common/constants.h"

// Functions

void memory_set_8(u8* destination, u8 value, u32 length);
void memory_set_16(u16* destination, u16 value, u32 length);
void memory_set_32(u32* destination, u32 value, u32 length);
void memory_set_64(u64* destination, u64 value, u32 length);

void memory_set_bit_8(void* destination, u8 value, u32 bytes);
void memory_set_bit_16(void* destination, u16 value, u32 bytes);
void memory_set_bit_32(void* destination, u32 value, u32 bytes);
void memory_set_bit_64(void* destination, u64 value, u32 bytes);

void memory_copy(void* source, void* destination, u32 bytes);

void memory_clear(const void* destination, u32 bytes);

#endif
