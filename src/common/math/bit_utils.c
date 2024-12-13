#include "bit_utils.h"

#include "common/constants.h"

// Bit Functions

u8 u8_rotate_bits_left(u8 value, u8 bits) {
    bits %= U8_BIT_COUNT;
    return (((value << bits) & 0xFF) | (value >> (U8_BIT_COUNT - bits)));
}

u8 u8_rotate_bits_right(u8 value, u8 bits) {
    bits %= U8_BIT_COUNT;
    return (((value >> bits) & 0xFF) | (value << (U8_BIT_COUNT - bits)));
}

u16 u16_rotate_bits_left(u16 value, u16 bits) {
    bits %= U16_BIT_COUNT;
    return ((value << bits) | (value >> (U16_BIT_COUNT - bits)));
}

u16 u16_rotate_bits_right(u16 value, u16 bits) {
    bits %= U16_BIT_COUNT;
    return ((value >> bits) | (value << (U16_BIT_COUNT - bits)));
}

u32 u32_rotate_bits_left(u32 value, u32 bits) {
    bits %= U32_BIT_COUNT;
    return (value << bits) | (value >> (U32_BIT_COUNT - bits));
}

u32 u32_rotate_bits_right(u32 value, u32 bits) {
    bits %= U32_BIT_COUNT;
    return (value >> bits) | (value << (U32_BIT_COUNT - bits));
}

u64 u64_rotate_bits_left(u64 value, u64 bits) {
    bits %= U64_BIT_COUNT;
    return (value << bits) | (value >> (U64_BIT_COUNT - bits));
}

u64 u64_rotate_bits_right(u64 value, u64 bits) {
    bits %= U64_BIT_COUNT;
    return (value >> bits) | (value << (U64_BIT_COUNT - bits));
}

u8 u32_count_leading_zeros(u32 num) {
    static const u8 debruijn32[32] = { 0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19, 1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18 };

    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;

    num++;

    return debruijn32[(num * 0b00000111011010111110011000101001) >> 27];
}

u8 u32_lowest_bit_index(u32 num) {
    if (num == 0) {
        return U8_MAX;
    }

    static const u8 bit_position_lookup[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };
    return bit_position_lookup[((u32) ((num & -num) * 0b00000111011111001011010100110001)) >> 27];
}

u8 u32_highest_bit_index(u32 num) {
    if (num == 0) {
        return U8_MAX;
    }

    static const u8 bit_position_lookup[32] = { 0, 1, 16, 2, 29, 17, 3, 22, 30, 20, 18, 11, 13, 4, 7, 23, 31, 15, 28, 21, 19, 10, 12, 6, 14, 27, 9, 5, 26, 8, 25, 24 };

    num |= (num >> 1);
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);

    num = num - (num >> 1);

    return bit_position_lookup[(num * 0b00000110111010110001010011111001) >> 27];
}
