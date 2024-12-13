#ifndef STANDARD_LIBRARY_BIT_UTILS
#define STANDARD_LIBRARY_BIT_UTILS

// Includes

#include "common/constants.h"

// Typedefs

typedef u8* bitarray;

typedef u8* bitarray8;
typedef u16* bitarray16;
typedef u32* bitarray32;
typedef u64* bitarray64;

// Defines

#define BITARRAY_SET_BIT(array, index)   ((array)[(index) / 8] |= (0b1 << ((index) % 8)))               /* Set BIT in bitarray to 1 */
#define BITARRAY_CLEAR_BIT(array, index) ((array)[(index) / 8] &= ~(0b1 << ((index) % 8)))              /* Set BIT in bitarray to 0 */
#define BITARRAY_GET_BIT(array, index)   (((array)[(index) / 8] & (0b1 << ((index) % 8))) != 0)         /* Get BIT from bitarray */

#define BITARRAY8_SET_BIT(array, index)   BITARRAY_SET_BIT(array, index)
#define BITARRAY8_CLEAR_BIT(array, index) BITARRAY_CLEAR_BIT(array, index)
#define BITARRAY8_GET_BIT(array, index)  BITARRAY_GET_BIT(array, index)

#define BITARRAY16_SET_BIT(array, index)  ((array)[(index) / 16] |= (0b1 << ((index) % 16)))            /* Set BIT in bitarray to 1 */
#define BITARRAY16_CLEAR_BIT(array, index) ((array)[(index) / 16] &= ~(0b1 << ((index) % 16)))          /* Set BIT in bitarray to 0 */
#define BITARRAY16_GET_BIT(array, index)   (((array)[(index) / 16] & (0b1 << ((index) % 16))) != 0)     /* Get BIT from bitarray */

#define BITARRAY32_SET_BIT(array, index)   ((array)[(index) / 32] |= (0b1 << ((index) % 32)))           /* Set BIT in bitarray to 1 */
#define BITARRAY32_CLEAR_BIT(array, index) ((array)[(index) / 32] &= ~(0b1 << ((index) % 32)))          /* Set BIT in bitarray to 0 */
#define BITARRAY32_GET_BIT(array, index)   (((array)[(index) / 32] & (0b1 << ((index) % 32))) != 0)     /* Get BIT from bitarray */

#define BITARRAY64_SET_BIT(array, index)   ((array)[(index) / 64] |= (0b1 << ((index) % 64)))           /* Set BIT in bitarray to 1 */
#define BITARRAY64_CLEAR_BIT(array, index) ((array)[(index) / 64] &= ~(0b1 << ((index) % 64)))          /* Set BIT in bitarray to 0 */
#define BITARRAY64_GET_BIT(array, index)   (((array)[(index) / 64] & (0b1 << ((index) % 64))) != 0)     /* Get BIT from bitarray */

#define BYTESWAP8(value) (value)

#define BYTESWAP16(value) ((((value) >> 8) & 0xFF) | (((value) & 0xFF) << 8))

#define BYTESWAP32(value)                                               \
    (                                                                   \
    (((value) & 0xFF000000) >> 24) | (((value) & 0x00FF0000) >>  8) |   \
    (((value) & 0x0000FF00) <<  8) | (((value) & 0x000000FF) << 24)     \
    )

#define BYTESWAP64(value)                       \
    (                                           \
    (((value) & 0xFF00000000000000ULL) >> 56) | \
    (((value) & 0x00FF000000000000ULL) >> 40) | \
    (((value) & 0x0000FF0000000000ULL) >> 24) | \
    (((value) & 0x000000FF00000000ULL) >> 8)  | \
    (((value) & 0x00000000FF000000ULL) << 8)  | \
    (((value) & 0x0000000000FF0000ULL) << 24) | \
    (((value) & 0x000000000000FF00ULL) << 40) | \
    (((value) & 0x00000000000000FFULL) << 56)   \
    )

// Bit Functions

u8 u8_rotate_bits_left(u8 value, u8 bits);
u8 u8_rotate_bits_right(u8 value, u8 bits);

u16 u16_rotate_bits_left(u16 value, u16 bits);
u16 u16_rotate_bits_right(u16 value, u16 bits);

u32 u32_rotate_bits_left(u32 value, u32 bits);
u32 u32_rotate_bits_right(u32 value, u32 bits);

u64 u64_rotate_bits_left(u64 value, u64 bits);
u64 u64_rotate_bits_right(u64 value, u64 bits);

u8 u32_count_leading_zeros(u32 num);
u8 u32_lowest_bit_index(u32 num);
u8 u32_highest_bit_index(u32 num);

#endif
