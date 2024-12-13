#include "vec4f.h"

#include "common/constants.h"

#include "common/math/primitives/f32_math.h"

// Vec4f Functions

// Length & Length Squared

f32 vec4f_length(vec4f this) {
    return f32_sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
}

f32 vec4f_length_squared(vec4f this) {
    return this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w;
}

// Adding, Subtracting & Multiplying

vec4f* vec4f_add(vec4f* this, vec4f vec) {
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    this->w += vec.w;

    return this;
}

vec4f* vec4f_subtract(vec4f* this, vec4f vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    this->w -= vec.w;

    return this;
}

vec4f* vec4f_multiply(vec4f* this, vec4f vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;
    this->w *= vec.w;

    return this;
}

vec4f* vec4f_add_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w) {
    this->x += x;
    this->y += y;
    this->z += z;
    this->w += w;

    return this;
}

vec4f* vec4f_subtract_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w) {
    this->x -= x;
    this->y -= y;
    this->z -= z;
    this->w -= w;

    return this;
}

vec4f* vec4f_multiply_xyz(vec4f* this, f32 x, f32 y, f32 z, f32 w) {
    this->x *= x;
    this->y *= y;
    this->z *= z;
    this->w *= w;

    return this;
}

vec4f vec4f_add_n(vec4f this, vec4f vec) {
    return (vec4f) { this.x + vec.x, this.y + vec.y, this.z + vec.z, this.w + vec.w };
}

vec4f vec4f_subtract_n(vec4f this, vec4f vec) {
    return (vec4f) { this.x - vec.x, this.y - vec.y, this.z - vec.z, this.w - vec.w };
}

vec4f vec4f_multiply_n(vec4f this, vec4f vec) {
    return (vec4f) { this.x * vec.x, this.y * vec.y, this.z * vec.z, this.w * vec.w };
}

vec4f vec4f_add_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w) {
    return (vec4f) { this.x + x, this.y + y, this.z + z, this.w + w };
}

vec4f vec4f_subtract_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w) {
    return (vec4f) { this.x - x, this.y - y, this.z - z, this.w - w };
}

vec4f vec4f_multiply_n_xyz(vec4f this, f32 x, f32 y, f32 z, f32 w) {
    return (vec4f) { this.x * x, this.y * y, this.z * z, this.w * w };
}

// Other Functions

vec4f* vec4f_relativize(vec4f* this, vec4f vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;
    this->z = this->z - vec.z;
    this->w = this->w - vec.w;

    return this;
}

vec4f vec4f_relativize_n(vec4f this, vec4f vec) {
    return (vec4f) { this.x - vec.x, this.y - vec.y, this.z - vec.z, this.w - vec.w };
}

vec4f* vec4f_normalize(vec4f* this) {
    f32 distance = f32_sqrt(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);

    if (distance < 1.0E-4) {
        this->x = 0;
        this->y = 0;
        this->z = 0;
        this->w = 0;
    } else {
        this->x = this->x / distance;
        this->y = this->y / distance;
        this->z = this->z / distance;
        this->w = this->w / distance;
    }

    return this;
}

vec4f vec4f_normalize_n(vec4f this) {
    f32 distance = f32_sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
    return distance < 1.0E-4 ? VEC4F_ZERO : (vec4f) { this.x / distance, this.y / distance, this.z / distance, this.w / distance };
}

vec4f* vec4f_negate(vec4f* this) {
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;
    this->w = -this->w;

    return this;
}

vec4f vec4f_negate_n(vec4f this) {
    return (vec4f) { -this.x, -this.y, -this.z, -this.w };
}

bool vec4f_equals(vec4f this, vec4f vec) {
    return this.x == vec.x && this.y == vec.y && this.z == vec.z && this.w == vec.w;
}
