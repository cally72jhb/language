#ifndef STANDARD_LIBRARY_VEC3I
#define STANDARD_LIBRARY_VEC3I

// Includes

#include "common/constants.h"

#include "common/math/vec/vec3d.h"

// Typedefs

typedef struct vec3i {
    union {
        struct {
            i32 x;
            i32 y;
            i32 z;
        };

        i32 values[3];
    };
} vec3i, block_pos;

// Defines

#define VEC3I_ZERO ((vec3i) { 0, 0, 0 })

// Vec3i Functions

// Distance, Distance from Center, Manhattan Distance & Cross Product

f64 vec3i_distance(vec3i this, vec3i vec);
f64 vec3i_distance_f64(vec3i this, vec3d vec);
f64 vec3i_distance_f64_xyz(vec3i this, f64 x, f64 y, f64 z);

i32 vec3i_distance_squared(vec3i this, vec3i vec);
f64 vec3i_distance_squared_f64(vec3i this, vec3d vec);
f64 vec3i_distance_squared_f64_xyz(vec3i this, f64 x, f64 y, f64 z);

f64 vec3i_distance_from_center(vec3i this, vec3i vec);
f64 vec3i_distance_from_center_f64(vec3i this, vec3d vec);
f64 vec3i_distance_from_center_f64_xyz(vec3i this, f64 x, f64 y, f64 z);

f64 vec3i_distance_squared_from_center(vec3i this, vec3i vec);
f64 vec3i_distance_squared_from_center_f64(vec3i this, vec3d vec);
f64 vec3i_distance_squared_from_center_f64_xyz(vec3i this, f64 x, f64 y, f64 z);

i32 vec3i_manhattan_distance(vec3i this, vec3i vec);

vec3i vec3i_cross_product(vec3i this, vec3i vec);

// Adding, Subtracting & Multiplying

vec3i* vec3i_add(vec3i* this, vec3i vec);
vec3i* vec3i_subtract(vec3i* this, vec3i vec);
vec3i* vec3i_multiply(vec3i* this, vec3i vec);

vec3i* vec3i_add_xyz(vec3i* this, i32 x, i32 y, i32 z);
vec3i* vec3i_subtract_xyz(vec3i* this, i32 x, i32 y, i32 z);
vec3i* vec3i_multiply_xyz(vec3i* this, i32 x, i32 y, i32 z);

vec3i* vec3i_add_f64_xyz(vec3i* this, f64 x, f64 y, f64 z);
vec3i* vec3i_subtract_f64_xyz(vec3i* this, f64 x, f64 y, f64 z);
vec3i* vec3i_multiply_f64_xyz(vec3i* this, f64 x, f64 y, f64 z);

vec3i vec3i_add_n(vec3i this, vec3i vec);
vec3i vec3i_subtract_n(vec3i this, vec3i vec);
vec3i vec3i_multiply_n(vec3i this, vec3i vec);

vec3i vec3i_add_n_xyz(vec3i this, i32 x, i32 y, i32 z);
vec3i vec3i_subtract_n_xyz(vec3i this, i32 x, i32 y, i32 z);
vec3i vec3i_multiply_n_xyz(vec3i this, i32 x, i32 y, i32 z);

vec3i vec3i_add_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z);
vec3i vec3i_subtract_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z);
vec3i vec3i_multiply_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z);

// Other Functions

vec3i* vec3i_negate(vec3i* this);
vec3i vec3i_negate_n(vec3i this);

bool vec3i_equals(vec3i this, vec3i vec);

#endif
