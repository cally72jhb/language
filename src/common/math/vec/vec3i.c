#include "vec3i.h"

#include "common/constants.h"

#include "common/math/primitives/f64_math.h"

#include "common/math/vec/vec3d.h"

// Vec3i Functions

// Dot-/Cross Product, Distance & Length

f64 vec3i_distance(vec3i this, vec3i vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;
    f64 dz = this.z - vec.z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3i_distance_f64(vec3i this, vec3d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;
    f64 dz = this.z - vec.z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3i_distance_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    f64 dx = this.x - x;
    f64 dy = this.y - y;
    f64 dz = this.z - z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

i32 vec3i_distance_squared(vec3i this, vec3i vec) {
    i32 dx = this.x - vec.x;
    i32 dy = this.y - vec.y;
    i32 dz = this.z - vec.z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3i_distance_squared_f64(vec3i this, vec3d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;
    f64 dz = this.z - vec.z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3i_distance_squared_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    f64 dx = this.x - x;
    f64 dy = this.y - y;
    f64 dz = this.z - z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3i_distance_from_center(vec3i this, vec3i vec) {
    f64 dx = this.x + 0.5 - vec.x + 0.5;
    f64 dy = this.y + 0.5 - vec.y + 0.5;
    f64 dz = this.z + 0.5 - vec.z + 0.5;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3i_distance_from_center_f64(vec3i this, vec3d vec) {
    f64 dx = this.x + 0.5 - vec.x;
    f64 dy = this.y + 0.5 - vec.y;
    f64 dz = this.z + 0.5 - vec.z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3i_distance_from_center_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    f64 dx = this.x + 0.5 - x;
    f64 dy = this.y + 0.5 - y;
    f64 dz = this.z + 0.5 - z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3i_distance_squared_from_center(vec3i this, vec3i vec) {
    f64 dx = this.x + 0.5 - vec.x + 0.5;
    f64 dy = this.y + 0.5 - vec.y + 0.5;
    f64 dz = this.z + 0.5 - vec.z + 0.5;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3i_distance_squared_from_center_f64(vec3i this, vec3d vec) {
    f64 dx = this.x + 0.5 - vec.x;
    f64 dy = this.y + 0.5 - vec.y;
    f64 dz = this.z + 0.5 - vec.z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3i_distance_squared_from_center_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    f64 dx = this.x + 0.5 - x;
    f64 dy = this.y + 0.5 - y;
    f64 dz = this.z + 0.5 - z;

    return dx * dx + dy * dy + dz * dz;
}

i32 vec3i_manhattan_distance(vec3i this, vec3i vec) {
    f32 x = (f32) (this.x < vec.x ? vec.x - this.x : this.x - vec.x);
    f32 y = (f32) (this.y < vec.y ? vec.y - this.y : this.y - vec.y);
    f32 z = (f32) (this.z < vec.z ? vec.z - this.z : this.z - vec.z);

    return (i32) (x + y + z);
}

vec3i vec3i_cross_product(vec3i this, vec3i vec) {
    return (vec3i) { this.y * vec.z - this.z * vec.y, this.z * vec.x - this.x * vec.z, this.x * vec.y - this.y * vec.x };
}

// Adding, Subtracting & Multiplying

vec3i* vec3i_add(vec3i* this, vec3i vec) {
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;

    return this;
}

vec3i* vec3i_subtract(vec3i* this, vec3i vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;

    return this;
}

vec3i* vec3i_multiply(vec3i* this, vec3i vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;

    return this;
}

vec3i* vec3i_add_xyz(vec3i* this, i32 x, i32 y, i32 z) {
    this->x += x;
    this->y += y;
    this->z += z;

    return this;
}

vec3i* vec3i_subtract_xyz(vec3i* this, i32 x, i32 y, i32 z) {
    this->x -= x;
    this->y -= y;
    this->z -= z;

    return this;
}

vec3i* vec3i_multiply_xyz(vec3i* this, i32 x, i32 y, i32 z) {
    this->x *= x;
    this->y *= y;
    this->z *= z;

    return this;
}

vec3i* vec3i_add_f64_xyz(vec3i* this, f64 x, f64 y, f64 z) {
    this->x = (i32) (this->x + x);
    this->y = (i32) (this->y + y);
    this->z = (i32) (this->z + z);

    return this;
}

vec3i* vec3i_subtract_f64_xyz(vec3i* this, f64 x, f64 y, f64 z) {
    this->x = (i32) (this->x - x);
    this->y = (i32) (this->y - y);
    this->z = (i32) (this->z - z);

    return this;
}

vec3i* vec3i_multiply_f64_xyz(vec3i* this, f64 x, f64 y, f64 z) {
    this->x = (i32) (this->x * x);
    this->y = (i32) (this->y * y);
    this->z = (i32) (this->z * z);

    return this;
}

vec3i vec3i_add_n(vec3i this, vec3i vec) {
    return (vec3i) { this.x + vec.x, this.y + vec.y, this.z + vec.z };
}

vec3i vec3i_subtract_n(vec3i this, vec3i vec) {
    return (vec3i) { this.x - vec.x, this.y - vec.y, this.z - vec.z };
}

vec3i vec3i_multiply_n(vec3i this, vec3i vec) {
    return (vec3i) { this.x * vec.x, this.y * vec.y, this.z * vec.z };
}

vec3i vec3i_add_n_xyz(vec3i this, i32 x, i32 y, i32 z) {
    return (vec3i) { this.x + x, this.y + y, this.z + z };
}

vec3i vec3i_subtract_n_xyz(vec3i this, i32 x, i32 y, i32 z) {
    return (vec3i) { this.x - x, this.y - y, this.z - z };
}

vec3i vec3i_multiply_n_xyz(vec3i this, i32 x, i32 y, i32 z) {
    return (vec3i) { this.x * x, this.y * y, this.z * z };
}

vec3i vec3i_add_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    return (vec3i) { (i32) (this.x + x), (i32) (this.y + y), (i32) (this.z + z) };
}

vec3i vec3i_subtract_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    return (vec3i) { (i32) (this.x - x), (i32) (this.y - y), (i32) (this.z - z) };
}

vec3i vec3i_multiply_n_f64_xyz(vec3i this, f64 x, f64 y, f64 z) {
    return (vec3i) { (i32) (this.x * x), (i32) (this.y * y), (i32) (this.z * z) };
}

// Other Functions

vec3i* vec3i_negate(vec3i* this) {
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;

    return this;
}

vec3i vec3i_negate_n(vec3i this) {
    return (vec3i) { -this.x, -this.y, -this.z };
}

bool vec3i_equals(vec3i this, vec3i vec) {
    return this.x == vec.x && this.y == vec.y && this.z == vec.z;
}
