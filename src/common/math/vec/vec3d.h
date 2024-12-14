#ifndef STANDARD_LIBRARY_VEC3D
#define STANDARD_LIBRARY_VEC3D

// Includes

#include "common/constants.h"

// Typedefs

typedef struct {
    union {
        struct {
            f64 x;
            f64 y;
            f64 z;
        };

        f64 values[3];
    };
} vec3d;

// Defines

#define VEC3D_ZERO ((vec3d) { 0.0, 0.0, 0.0 })

// Vec3d Functions

// Dot-/Cross Product, Distance & Length

f64 vec3d_dot_product(vec3d this, vec3d vec);
vec3d vec3d_cross_product(vec3d this, vec3d vec);
f64 vec3d_distance(vec3d this, vec3d vec);
f64 vec3d_distance_xyz(vec3d this, f64 x, f64 y, f64 z);
f64 vec3d_distance_squared(vec3d this, vec3d vec);
f64 vec3d_distance_squared_xyz(vec3d this, f64 x, f64 y, f64 z);

f64 vec3d_length(vec3d this);
f64 vec3d_length_squared(vec3d this);
f64 vec3d_horizontal_length(vec3d this);
f64 vec3d_horizontal_length_squared(vec3d this);

// Adding, Subtracting & Multiplying

vec3d* vec3d_add(vec3d* this, vec3d vec);
vec3d* vec3d_subtract(vec3d* this, vec3d vec);
vec3d* vec3d_multiply(vec3d* this, vec3d vec);
vec3d* vec3d_add_xyz(vec3d* this, f64 x, f64 y, f64 z);
vec3d* vec3d_subtract_xyz(vec3d* this, f64 x, f64 y, f64 z);
vec3d* vec3d_multiply_xyz(vec3d* this, f64 x, f64 y, f64 z);

vec3d vec3d_add_n(vec3d this, vec3d vec);
vec3d vec3d_subtract_n(vec3d this, vec3d vec);
vec3d vec3d_multiply_n(vec3d this, vec3d vec);
vec3d vec3d_add_n_xyz(vec3d this, f64 x, f64 y, f64 z);
vec3d vec3d_subtract_n_xyz(vec3d this, f64 x, f64 y, f64 z);
vec3d vec3d_multiply_n_xyz(vec3d this, f64 x, f64 y, f64 z);

// Rotating

vec3d* vec3d_rotate_x(vec3d* this, f64 angle);
vec3d* vec3d_rotate_y(vec3d* this, f64 angle);
vec3d* vec3d_rotate_z(vec3d* this, f64 angle);

vec3d vec3d_rotate_n_x(vec3d this, f64 angle);
vec3d vec3d_rotate_n_y(vec3d this, f64 angle);
vec3d vec3d_rotate_n_z(vec3d this, f64 angle);

// Other Functions

vec3d* vec3d_relativize(vec3d* this, vec3d vec);
vec3d vec3d_relativize_n(vec3d this, vec3d vec);
vec3d* vec3d_normalize(vec3d* this);
vec3d vec3d_normalize_n(vec3d this);

vec3d* vec3d_negate(vec3d* this);
vec3d vec3d_negate_n(vec3d this);

bool vec3d_equals(vec3d this, vec3d vec);

#endif
