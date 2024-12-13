#include "vec3d.h"

#include "common/constants.h"

#include "common/math/primitives/f64_math.h"

// Vec3d Functions

// Dot-/Cross Product, Distance & Length

f64 vec3d_dot_product(vec3d this, vec3d vec) {
    return this.x * vec.x + this.y * vec.y + this.z * vec.z;
}

vec3d vec3d_cross_product(vec3d this, vec3d vec) {
    return (vec3d) { this.y * vec.z - this.z * vec.y, this.z * vec.x - this.x * vec.z, this.x * vec.y - this.y * vec.x };
}

f64 vec3d_distance(vec3d this, vec3d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;
    f64 dz = this.z - vec.z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3d_distance_xyz(vec3d this, f64 x, f64 y, f64 z) {
    f64 dx = this.x - x;
    f64 dy = this.y - y;
    f64 dz = this.z - z;

    return f64_sqrt(dx * dx + dy * dy + dz * dz);
}

f64 vec3d_distance_squared(vec3d this, vec3d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;
    f64 dz = this.z - vec.z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3d_distance_squared_xyz(vec3d this, f64 x, f64 y, f64 z) {
    f64 dx = this.x - x;
    f64 dy = this.y - y;
    f64 dz = this.z - z;

    return dx * dx + dy * dy + dz * dz;
}

f64 vec3d_length(vec3d this) {
    return f64_sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
}

f64 vec3d_length_squared(vec3d this) {
    return this.x * this.x + this.y * this.y + this.z * this.z;
}

f64 vec3d_horizontal_length(vec3d this) {
    return f64_sqrt(this.x * this.x + this.z * this.z);
}

f64 vec3d_horizontal_length_squared(vec3d this) {
    return this.x * this.x + this.z * this.z;
}

// Adding, Subtracting & Multiplying

vec3d* vec3d_add(vec3d* this, vec3d vec) {
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;

    return this;
}

vec3d* vec3d_subtract(vec3d* this, vec3d vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;

    return this;
}

vec3d* vec3d_multiply(vec3d* this, vec3d vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;

    return this;
}

vec3d* vec3d_add_xyz(vec3d* this, f64 x, f64 y, f64 z) {
    this->x += x;
    this->y += y;
    this->z += z;

    return this;
}

vec3d* vec3d_subtract_xyz(vec3d* this, f64 x, f64 y, f64 z) {
    this->x -= x;
    this->y -= y;
    this->z -= z;

    return this;
}

vec3d* vec3d_multiply_xyz(vec3d* this, f64 x, f64 y, f64 z) {
    this->x *= x;
    this->y *= y;
    this->z *= z;

    return this;
}

vec3d vec3d_add_n(vec3d this, vec3d vec) {
    return (vec3d) { this.x + vec.x, this.y + vec.y, this.z + vec.z };
}

vec3d vec3d_subtract_n(vec3d this, vec3d vec) {
    return (vec3d) { this.x - vec.x, this.y - vec.y, this.z - vec.z };
}

vec3d vec3d_multiply_n(vec3d this, vec3d vec) {
    return (vec3d) { this.x * vec.x, this.y * vec.y, this.z * vec.z };
}

vec3d vec3d_add_n_xyz(vec3d this, f64 x, f64 y, f64 z) {
    return (vec3d) { this.x + x, this.y + y, this.z + z };
}

vec3d vec3d_subtract_n_xyz(vec3d this, f64 x, f64 y, f64 z) {
    return (vec3d) { this.x - x, this.y - y, this.z - z };
}

vec3d vec3d_multiply_n_xyz(vec3d this, f64 x, f64 y, f64 z) {
    return (vec3d) { this.x * x, this.y * y, this.z * z };
}

// Rotating

vec3d* vec3d_rotate_x(vec3d* this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    this->x = this->x;
    this->y = this->y * (f64) cos + this->z * (f64) sin;
    this->z = this->z * (f64) cos - this->y * (f64) sin;

    return this;
}

vec3d* vec3d_rotate_y(vec3d* this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    this->x = this->x * (f64) cos + this->z * (f64) sin;
    this->y = this->y;
    this->z = this->z * (f64) cos - this->x * (f64) sin;

    return this;
}

vec3d* vec3d_rotate_z(vec3d* this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    this->x = this->x * (f64) cos + this->y * (f64) sin;
    this->y = this->y * (f64) cos - this->x * (f64) sin;
    this->z = this->z;

    return this;
}

vec3d vec3d_rotate_n_x(vec3d this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    f64 x = this.x;
    f64 y = this.y * (f64) cos + this.z * (f64) sin;
    f64 z = this.z * (f64) cos - this.y * (f64) sin;

    return (vec3d) { x, y, z };
}

vec3d vec3d_rotate_n_y(vec3d this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    f64 x = this.x * (f64) cos + this.z * (f64) sin;
    f64 y = this.y;
    f64 z = this.z * (f64) cos - this.x * (f64) sin;

    return (vec3d) { x, y, z };
}

vec3d vec3d_rotate_n_z(vec3d this, f64 angle) {
    f64 sin = f64_sin(angle);
    f64 cos = f64_cos(angle);

    f64 x = this.x * (f64) cos + this.y * (f64) sin;
    f64 y = this.y * (f64) cos - this.x * (f64) sin;
    f64 z = this.z;

    return (vec3d) { x, y, z };
}

// Other Functions

vec3d* vec3d_relativize(vec3d* this, vec3d vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;
    this->z = this->z - vec.z;

    return this;
}

vec3d vec3d_relativize_n(vec3d this, vec3d vec) {
    return (vec3d) { this.x - vec.x, this.y - vec.y, this.z - vec.z };
}

vec3d* vec3d_normalize(vec3d* this) {
    f64 distance = f64_sqrt(this->x * this->x + this->y * this->y + this->z * this->z);

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

vec3d vec3d_normalize_n(vec3d this) {
    f64 distance = f64_sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
    return distance < 1.0E-4 ? VEC3D_ZERO : (vec3d) { this.x / distance, this.y / distance, this.z / distance };
}

vec3d* vec3d_negate(vec3d* this) {
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;

    return this;
}

vec3d vec3d_negate_n(vec3d this) {
    return (vec3d) { -this.x, -this.y, -this.z };
}

bool vec3d_equals(vec3d this, vec3d vec) {
    return this.x == vec.x && this.y == vec.y && this.z == vec.z;
}
