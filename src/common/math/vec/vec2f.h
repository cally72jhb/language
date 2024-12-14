#ifndef STANDARD_LIBRARY_VEC2F
#define STANDARD_LIBRARY_VEC2F

// Includes

#include "common/constants.h"

// Typedefs

typedef struct {
    union {
        struct {
            f32 x;
            f32 y;
        };

        f32 values[2];
    };
} vec2f;

// Defines

#define VEC2F_ZERO ((vec2f) { 0.0F, 0.0F })

// Vec2f Functions

// Dot-/Cross Product, Distance & Length

f32 vec2f_dot_product(vec2f this, vec2f vec);
f32 vec2f_distance(vec2f this, vec2f vec);
f32 vec2f_distance_squared(vec2f this, vec2f vec);

f32 vec2f_length(vec2f this);
f32 vec2f_length_squared(vec2f this);

// Adding, Subtracting & Multiplying

vec2f* vec2f_add(vec2f* this, vec2f vec);
vec2f* vec2f_subtract(vec2f* this, vec2f vec);
vec2f* vec2f_multiply(vec2f* this, vec2f vec);
vec2f* vec2f_add_xy(vec2f* this, f32 x, f32 y);
vec2f* vec2f_subtract_xy(vec2f* this, f32 x, f32 y);
vec2f* vec2f_multiply_xy(vec2f* this, f32 x, f32 y);

vec2f vec2f_add_n(vec2f this, vec2f vec);
vec2f vec2f_subtract_n(vec2f this, vec2f vec);
vec2f vec2f_multiply_n(vec2f this, vec2f vec);
vec2f vec2f_add_n_xy(vec2f this, f32 x, f32 y);
vec2f vec2f_subtract_n_xy(vec2f this, f32 x, f32 y);
vec2f vec2f_multiply_n_xy(vec2f this, f32 x, f32 y);

// Other Functions

vec2f* vec2f_relativize(vec2f* this, vec2f vec);
vec2f vec2f_relativize_n(vec2f this, vec2f vec);
vec2f* vec2f_normalize(vec2f* this);
vec2f vec2f_normalize_n(vec2f this);

vec2f* vec2f_negate(vec2f* this);
vec2f vec2f_negate_n(vec2f this);

bool vec2f_equals(vec2f this, vec2f vec);

#endif
