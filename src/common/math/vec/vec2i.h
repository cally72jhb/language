#ifndef STANDARD_LIBRARY_VEC2I
#define STANDARD_LIBRARY_VEC2I

// Includes

#include "common/constants.h"

// Typedefs

typedef struct vec2i {
    union {
        struct {
            i32 x;
            i32 y;
        };

        i32 values[2];
    };
} vec2i;

// Defines

#define VEC2I_ZERO ((vec2i) { 0, 0 })

// Vec2i Functions

// Dot-/Cross Product, Distance & Length

i32 vec2i_dot_product(vec2i this, vec2i vec);
f64 vec2i_distance(vec2i this, vec2i vec);
i32 vec2i_distance_squared(vec2i this, vec2i vec);

f64 vec2i_length(vec2i this);
i32 vec2i_length_squared(vec2i this);

// Adding, Subtracting & Multiplying

vec2i* vec2i_add(vec2i* this, vec2i vec);
vec2i* vec2i_subtract(vec2i* this, vec2i vec);
vec2i* vec2i_multiply(vec2i* this, vec2i vec);
vec2i* vec2i_add_xy(vec2i* this, i32 x, i32 y);
vec2i* vec2i_subtract_xy(vec2i* this, i32 x, i32 y);
vec2i* vec2i_multiply_xy(vec2i* this, i32 x, i32 y);

vec2i vec2i_add_n(vec2i this, vec2i vec);
vec2i vec2i_subtract_n(vec2i this, vec2i vec);
vec2i vec2i_multiply_n(vec2i this, vec2i vec);
vec2i vec2i_add_n_xy(vec2i this, i32 x, i32 y);
vec2i vec2i_subtract_n_xy(vec2i this, i32 x, i32 y);
vec2i vec2i_multiply_n_xy(vec2i this, i32 x, i32 y);

// Other Functions

vec2i* vec2i_relativize(vec2i* this, vec2i vec);
vec2i vec2i_relativize_n(vec2i this, vec2i vec);
vec2i* vec2i_normalize(vec2i* this);
vec2i vec2i_normalize_n(vec2i this);

vec2i* vec2i_negate(vec2i* this);
vec2i vec2i_negate_n(vec2i this);

bool vec2i_equals(vec2i this, vec2i vec);

#endif
