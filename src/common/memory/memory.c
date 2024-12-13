#include "memory.h"

#include "common/constants.h"
#include "common/defines.h"

void memory_set_8(u8* destination, u8 value, u32 length) {
    u8* destination_end = destination + length;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = value; value_part |= value_part << U8_BIT_COUNT; value_part |= value_part << U16_BIT_COUNT; value_part |= value_part << U32_BIT_COUNT;
    while ((destination_end - destination) >= sizeof(u64)) {
        *((u64*) destination) = value_part;
        destination += sizeof(u64);
    }
    #else
    u32 value_part = value; value_part |= value_part << U8_BIT_COUNT; value_part |= value_part << U16_BIT_COUNT;
    while ((destination_end - destination) >= sizeof(u32)) {
        *((u32*) destination) = value_part;
        destination += sizeof(u32);
    }
    #endif

    while ((destination_end - destination) > 0) {
        *((u8*) destination) = (u8) (value & 0xFF);
        destination += sizeof(byte);
    }
}

void memory_set_16(u16* destination, u16 value, u32 length) {
    u16* destination_end = destination + length;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = value; value_part |= value_part << U16_BIT_COUNT; value_part |= value_part << U32_BIT_COUNT;
    while ((destination_end - destination) >= sizeof(u64)) {
        *((u64*) destination) = value_part;
        destination += sizeof(u64) / sizeof(u16);
    }
    #else
    u32 value_part = (((u32) value) << U16_BIT_COUNT) | ((u64) value);
    while ((destination_end - destination) >= sizeof(u32)) {
        *((u32*) destination) = value_part;
        destination += sizeof(u32) / sizeof(u16);
    }
    #endif

    while ((destination_end - destination) > 0) {
        *destination = value;
        destination++;
    }
}

void memory_set_32(u32* destination, u32 value, u32 length) {
    u32* destination_end = destination + length;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = (((u64) value) << U32_BIT_COUNT) | ((u64) value);
    while ((destination_end - destination) >= sizeof(u64)) {
        *((u64*) destination) = value_part;
        destination += sizeof(u64) / sizeof(u32);
    }
    #else
    while ((destination_end - destination) >= sizeof(u32)) {
        *((u32*) destination) = value;
        destination += sizeof(u32) / sizeof(u32);
    }
    #endif

    while ((destination_end - destination) > 0) {
        *destination = value;
        destination++;
    }
}

void memory_set_64(u64* destination, u64 value, u32 length) {
    u64* destination_end = destination + length;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    while ((destination_end - destination) > 0) {
        *destination = value;
        destination++;
    }
    #else
    u32 value_part1 = value & ((u64) U32_BIT_1);
    u32 value_part2 = value >> U32_BIT_COUNT;
    while ((destination_end - destination) >= sizeof(u32)) {
        ((u32*) &destination)[0] = value_part1;
        ((u32*) &destination)[1] = value_part2;
        destination += sizeof(u32) * 2;
    }
    #endif
}

void memory_set_bit_8(void* destination, u8 value, u32 bytes) {
    byte* destination_temp = (byte*) destination;
    byte* destination_end = destination_temp + bytes;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = value; value_part |= value_part << U8_BIT_COUNT; value_part |= value_part << U16_BIT_COUNT; value_part |= value_part << U32_BIT_COUNT;
    while ((destination_end - destination_temp) >= sizeof(u64)) {
        *((u64*) destination_temp) = value_part;
        destination_temp += sizeof(u64);
    }
    #else
    u32 value_part = value; value_part |= value_part << U8_BIT_COUNT; value_part |= value_part << U16_BIT_COUNT;
    while ((destination_end - destination_temp) >= sizeof(u32)) {
        *((u32*) destination_temp) = value_part;
        destination_temp += sizeof(u32);
    }
    #endif

    while ((destination_end - destination_temp) > 0) {
        *((u8*) destination_temp) = (u8) (value & 0xFF);
        destination_temp += sizeof(byte);
    }
}

void memory_set_bit_16(void* destination, u16 value, u32 bytes) {
    byte* destination_temp = (byte*) destination;
    byte* destination_end = destination_temp + bytes;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = value; value_part |= value_part << U16_BIT_COUNT; value_part |= value_part << U32_BIT_COUNT;
    while ((destination_end - destination_temp) >= sizeof(u64)) {
        *((u64*) destination_temp) = value_part;
        destination_temp += sizeof(u64);
    }
    #else
    u32 value_part = (((u32) value) << U16_BIT_COUNT) | ((u64) value);
    while ((destination_end - destination_temp) >= sizeof(u32)) {
        *((u32*) destination_temp) = value_part;
        destination_temp += sizeof(u32);
    }
    #endif

    u32 i = 0;
    while ((destination_end - destination_temp) > 0) {
        *((u8*) destination_temp) = ((u8*) &value)[i % sizeof(u16)];
        destination_temp += sizeof(byte);
        i++;
    }
}

void memory_set_bit_32(void* destination, u32 value, u32 bytes) {
    byte* destination_temp = (byte*) destination;
    byte* destination_end = destination_temp + bytes;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    u64 value_part = (((u64) value) << U32_BIT_COUNT) | ((u64) value);
    while ((destination_end - destination_temp) >= sizeof(u64)) {
        *((u64*) destination_temp) = value_part;
        destination_temp += sizeof(u64);
    }
    #else
    while ((destination_end - destination_temp) >= sizeof(u32)) {
        *((u32*) destination_temp) = value;
        destination_temp += sizeof(u32);
    }
    #endif

    u32 i = 0;
    while ((destination_end - destination_temp) > 0) {
        *((u8*) destination_temp) = ((u8*) &value)[i % sizeof(u32)];
        destination_temp += sizeof(byte);
        i++;
    }
}

void memory_set_bit_64(void* destination, u64 value, u32 bytes) {
    byte* destination_temp = (byte*) destination;
    byte* destination_end = destination_temp + bytes;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    while ((destination_end - destination_temp) >= sizeof(u64)) {
        *((u64*) destination_temp) = value;
        destination_temp += sizeof(u64);
    }
    #else
    u32 value_part1 = value & ((u64) U32_BIT_1);
    u32 value_part2 = value >> U32_BIT_COUNT;
    while ((destination_end - destination_temp) >= sizeof(u32)) {
        ((u32*) &destination_temp)[0] = value_part1;
        ((u32*) &destination_temp)[1] = value_part2;
        destination_temp += sizeof(u32) * 2;
    }
    #endif

    u32 i = 0;
    while ((destination_end - destination_temp) > 0) {
        *((u8*) destination_temp) = ((u8*) &value)[i];
        destination_temp += sizeof(byte);
        i++;
    }
}

void memory_copy(void* source, void* destination, u32 bytes) {
    byte* source_temp = (byte*) source;
    byte* destination_temp = (byte*) destination;

    byte* source_end = source_temp + bytes;

    #if PROGRAM_FEATURE_32BIT_MODE == 0
    while ((source_end - source_temp) > sizeof(u64)) {
        *((u64*) destination_temp) = *((u64*) source_temp);
        source_temp += sizeof(u64);
        destination_temp += sizeof(u64);
    }
    #else
    while ((source_end - source_temp) > sizeof(u64)) {
        *((u64*) destination_temp) = *((u64*) source_temp);
        destination_temp += sizeof(u64);
        source_temp += sizeof(u64);
    }
    #endif

    while ((source_end - source_temp) > 0) {
        *destination_temp = *source_temp;
        source_temp++;
        destination_temp++;
    }
}

void memory_clear(const void* destination, u32 bytes) {
    memory_set_8((u8*) destination, 0, bytes);
}
