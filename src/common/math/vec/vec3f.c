#include "vec3f.h"

#include "common/constants.h"

#include "common/math/primitives/f32_math.h"

// Vec3f Functions

// Dot-/Cross Product, Distance & Length

f32 vec3f_dot_product(vec3f this, vec3f vec) {
    return this.x * vec.x + this.y * vec.y + this.z * vec.z;
}

vec3f vec3f_cross_product(vec3f this, vec3f vec) {
    return (vec3f) { this.y * vec.z - this.z * vec.y, this.z * vec.x - this.x * vec.z, this.x * vec.y - this.y * vec.x };
}

f32 vec3f_distance(vec3f this, vec3f vec) {
    f32 dx = this.x - vec.x;
    f32 dy = this.y - vec.y;
    f32 dz = this.z - vec.z;

    return f32_sqrt(dx * dx + dy * dy + dz * dz);
}

f32 vec3f_distance_xyz(vec3f this, f32 x, f32 y, f32 z) {
    f32 dx = this.x - x;
    f32 dy = this.y - y;
    f32 dz = this.z - z;

    return f32_sqrt(dx * dx + dy * dy + dz * dz);
}

f32 vec3f_distance_squared(vec3f this, vec3f vec) {
    f32 dx = this.x - vec.x;
    f32 dy = this.y - vec.y;
    f32 dz = this.z - vec.z;

    return dx * dx + dy * dy + dz * dz;
}

f32 vec3f_distance_squared_xyz(vec3f this, f32 x, f32 y, f32 z) {
    f32 dx = this.x - x;
    f32 dy = this.y - y;
    f32 dz = this.z - z;

    return dx * dx + dy * dy + dz * dz;
}

f32 vec3f_length(vec3f this) {
    return f32_sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
}

f32 vec3f_length_squared(vec3f this) {
    return this.x * this.x + this.y * this.y + this.z * this.z;
}

f32 vec3f_horizontal_length(vec3f this) {
    return f32_sqrt(this.x * this.x + this.z * this.z);
}

f32 vec3f_horizontal_length_squared(vec3f this) {
    return this.x * this.x + this.z * this.z;
}

// Adding, Subtracting & Multiplying

vec3f* vec3f_add(vec3f* this, vec3f vec) {
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;

    return this;
}

vec3f* vec3f_subtract(vec3f* this, vec3f vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;

    return this;
}

vec3f* vec3f_multiply(vec3f* this, vec3f vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;

    return this;
}

vec3f* vec3f_add_xyz(vec3f* this, f32 x, f32 y, f32 z) {
    this->x += x;
    this->y += y;
    this->z += z;

    return this;
}

vec3f* vec3f_subtract_xyz(vec3f* this, f32 x, f32 y, f32 z) {
    this->x -= x;
    this->y -= y;
    this->z -= z;

    return this;
}

vec3f* vec3f_multiply_xyz(vec3f* this, f32 x, f32 y, f32 z) {
    this->x *= x;
    this->y *= y;
    this->z *= z;

    return this;
}

vec3f vec3f_add_n(vec3f this, vec3f vec) {
    return (vec3f) { this.x + vec.x, this.y + vec.y, this.z + vec.z };
}

vec3f vec3f_subtract_n(vec3f this, vec3f vec) {
    return (vec3f) { this.x - vec.x, this.y - vec.y, this.z - vec.z };
}

vec3f vec3f_multiply_n(vec3f this, vec3f vec) {
    return (vec3f) { this.x * vec.x, this.y * vec.y, this.z * vec.z };
}

vec3f vec3f_add_n_xyz(vec3f this, f32 x, f32 y, f32 z) {
    return (vec3f) { this.x + x, this.y + y, this.z + z };
}

vec3f vec3f_subtract_n_xyz(vec3f this, f32 x, f32 y, f32 z) {
    return (vec3f) { this.x - x, this.y - y, this.z - z };
}

vec3f vec3f_multiply_n_xyz(vec3f this, f32 x, f32 y, f32 z) {
    return (vec3f) { this.x * x, this.y * y, this.z * z };
}

// Other Functions

vec3f* vec3f_relativize(vec3f* this, vec3f vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;
    this->z = this->z - vec.z;

    return this;
}

vec3f vec3f_relativize_n(vec3f this, vec3f vec) {
    return (vec3f) { this.x - vec.x, this.y - vec.y, this.z - vec.z };
}

vec3f* vec3f_normalize(vec3f* this) {
    f32 distance = f32_sqrt(this->x * this->x + this->y * this->y + this->z * this->z);

    if (distance < 1.0E-4) {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    } else {
        this->x = this->x / distance;
        this->y = this->y / distance;
        this->z = this->z / distance;
    }

    return this;
}

vec3f vec3f_normalize_n(vec3f this) {
    f32 norm = f32_sqrt(vec3f_dot_product(this, this));

    if (norm == 0.0F) {
        return VEC3F_ZERO;
    }

    norm = 1 / norm;

    return (vec3f) { this.x * norm, this.y * norm, this.z * norm };
}

vec3f* vec3f_negate(vec3f* this) {
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;

    return this;
}

vec3f vec3f_negate_n(vec3f this) {
    return (vec3f) { -this.x, -this.y, -this.z };
}

bool vec3f_equals(vec3f this, vec3f vec) {
    return this.x == vec.x && this.y == vec.y && this.z == vec.z;
}
