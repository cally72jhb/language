#ifndef STANDARD_LIBRARY_VEC4F
#define STANDARD_LIBRARY_VEC4F

// Includes

#include "common/constants.h"

// Typedefs

typedef struct vec4f {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        f32 values[4];
    };
} vec4f;

// Defines

#define VEC4F_ZERO ((vec4f) { 0.0F, 0.0F, 0.0F, 0.0F })

// Vec4f Functions

// Length & Length Squared

f32 vec4f_length(vec4f this);
f32 vec4f_length_squared(vec4f this);

// Adding, Subtracting & Multiplying

vec4f* vec4f_add(vec4f* this, vec4f vec);
vec4f* vec4f_subtract(vec4f* this, vec4f vec);
vec4f* vec4f_multiply(vec4f* this, vec4f vec);
vec4f* vec4f_add_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w);
vec4f* vec4f_subtract_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w);
vec4f* vec4f_multiply_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w);

vec4f vec4f_add_n(vec4f this, vec4f vec);
vec4f vec4f_subtract_n(vec4f this, vec4f vec);
vec4f vec4f_multiply_n(vec4f this, vec4f vec);
vec4f vec4f_add_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w);
vec4f vec4f_subtract_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w);
vec4f vec4f_multiply_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w);

// Other Functions

vec4f* vec4f_relativize(vec4f* this, vec4f vec);
vec4f vec4f_relativize_n(vec4f this, vec4f vec);
vec4f* vec4f_normalize(vec4f* this);
vec4f vec4f_normalize_n(vec4f this);

vec4f* vec4f_negate(vec4f* this);
vec4f vec4f_negate_n(vec4f this);

bool vec4f_equals(vec4f this, vec4f vec);

#endif
