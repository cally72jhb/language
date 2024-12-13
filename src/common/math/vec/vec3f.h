#ifndef STANDARD_LIBRARY_VEC3F
#define STANDARD_LIBRARY_VEC3F

// Includes

#include "common/constants.h"

// Typedefs

typedef struct vec3f {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
        };

        f32 values[3];
    };
} vec3f;

// Defines

#define VEC3F_ZERO ((vec3f) { 0.0F, 0.0F, 0.0F })

// Vec3f Functions

// Dot-/Cross Product, Distance & Length

f32 vec3f_dot_product(vec3f this, vec3f vec);
vec3f vec3f_cross_product(vec3f this, vec3f vec);
f32 vec3f_distance(vec3f this, vec3f vec);
f32 vec3f_distance_xyz(vec3f this, f32 x, f32 y, f32 z);
f32 vec3f_distance_squared(vec3f this, vec3f vec);
f32 vec3f_distance_squared_xyz(vec3f this, f32 x, f32 y, f32 z);

f32 vec3f_length(vec3f this);
f32 vec3f_length_squared(vec3f this);
f32 vec3f_horizontal_length(vec3f this);
f32 vec3f_horizontal_length_squared(vec3f this);

// Adding, Subtracting & Multiplying

vec3f* vec3f_add(vec3f* this, vec3f vec);
vec3f* vec3f_subtract(vec3f* this, vec3f vec);
vec3f* vec3f_multiply(vec3f* this, vec3f vec);
vec3f* vec3f_add_xyz(vec3f* this, f32 x, f32 y, f32 z);
vec3f* vec3f_subtract_xyz(vec3f* this, f32 x, f32 y, f32 z);
vec3f* vec3f_multiply_xyz(vec3f* this, f32 x, f32 y, f32 z);

vec3f vec3f_add_n(vec3f this, vec3f vec);
vec3f vec3f_subtract_n(vec3f this, vec3f vec);
vec3f vec3f_multiply_n(vec3f this, vec3f vec);
vec3f vec3f_add_n_xyz(vec3f this, f32 x, f32 y, f32 z);
vec3f vec3f_subtract_n_xyz(vec3f this, f32 x, f32 y, f32 z);
vec3f vec3f_multiply_n_xyz(vec3f this, f32 x, f32 y, f32 z);

// Other Functions

vec3f* vec3f_relativize(vec3f* this, vec3f vec);
vec3f vec3f_relativize_n(vec3f this, vec3f vec);
vec3f* vec3f_normalize(vec3f* this);
vec3f vec3f_normalize_n(vec3f this);

vec3f* vec3f_negate(vec3f* this);
vec3f vec3f_negate_n(vec3f this);

bool vec3f_equals(vec3f this, vec3f vec);

#endif
