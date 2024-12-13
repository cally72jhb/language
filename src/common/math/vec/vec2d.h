#ifndef STANDARD_LIBRARY_VEC2D
#define STANDARD_LIBRARY_VEC2D

// Includes

#include "common/constants.h"

// Typedefs

typedef struct vec2d {
    union {
        struct {
            f64 x;
            f64 y;
        };

        f64 values[2];
    };
} vec2d;

// Defines

#define VEC2D_ZERO ((vec2d) { 0.0, 0.0 })

// Vec2d Functions

// Dot-/Cross Product, Distance & Length

f64 vec2d_dot_product(vec2d this, vec2d vec);
f64 vec2d_distance(vec2d this, vec2d vec);
f64 vec2d_distance_squared(vec2d this, vec2d vec);

f64 vec2d_length(vec2d this);
f64 vec2d_length_squared(vec2d this);

// Adding, Subtracting & Multiplying

vec2d* vec2d_add(vec2d* this, vec2d vec);
vec2d* vec2d_subtract(vec2d* this, vec2d vec);
vec2d* vec2d_multiply(vec2d* this, vec2d vec);
vec2d* vec2d_add_xy(vec2d* this, f64 x, f64 y);
vec2d* vec2d_subtract_xy(vec2d* this, f64 x, f64 y);
vec2d* vec2d_multiply_xy(vec2d* this, f64 x, f64 y);

vec2d vec2d_add_n(vec2d this, vec2d vec);
vec2d vec2d_subtract_n(vec2d this, vec2d vec);
vec2d vec2d_multiply_n(vec2d this, vec2d vec);
vec2d vec2d_add_n_xy(vec2d this, f64 x, f64 y);
vec2d vec2d_subtract_n_xy(vec2d this, f64 x, f64 y);
vec2d vec2d_multiply_n_xy(vec2d this, f64 x, f64 y);

// Other Functions

vec2d* vec2d_relativize(vec2d* this, vec2d vec);
vec2d vec2d_relativize_n(vec2d this, vec2d vec);
vec2d* vec2d_normalize(vec2d* this);
vec2d vec2d_normalize_n(vec2d this);

vec2d* vec2d_negate(vec2d* this);
vec2d vec2d_negate_n(vec2d this);

bool vec2d_equals(vec2d this, vec2d vec);

#endif
