#ifndef STANDARD_LIBRARY_MAT4
#define STANDARD_LIBRARY_MAT4

// Includes

#include "common/constants.h"

#include "common/math/vec/vec3f.h"
#include "common/math/vec/vec4f.h"

// Typedefs

typedef struct {
    union {
        struct {
            f32 m00; f32 m01; f32 m02; f32 m03;
            f32 m10; f32 m11; f32 m12; f32 m13;
            f32 m20; f32 m21; f32 m22; f32 m23;
            f32 m30; f32 m31; f32 m32; f32 m33;
        };

        vec4f columns[4];
        f32 values[4][4];
    };
} mat4;

// Defines

#define MAT4_ZERO ((mat4) { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F })
#define MAT4_IDENTITY ((mat4) { 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F })

// #define MAT4_GET(mat, x, y) (mat.values[x * 4 + (y == 4 ? 0 : y)])

// Mat4 Functions

bool mat4_equals(mat4* this, mat4* mat);

void mat4_scale(mat4* this, f32 scale);
mat4 mat4_inverse(mat4* this);

#endif
