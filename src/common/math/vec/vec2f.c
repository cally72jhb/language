#include "vec2f.h"

#include "common/constants.h"

#include "common/math/primitives/f32_math.h"

// Vec2f Functions

// Dot Product, Distance & Length

f32 vec2f_dot_product(vec2f this, vec2f vec) {
    return this.x * vec.x + this.y * vec.y;
}

f32 vec2f_distance(vec2f this, vec2f vec) {
    f32 dx = this.x - vec.x;
    f32 dy = this.y - vec.y;

    return f32_sqrt(dx * dx + dy * dy);
}

f32 vec2f_distance_squared(vec2f this, vec2f vec) {
    f32 dx = this.x - vec.x;
    f32 dy = this.y - vec.y;

    return dx * dx + dy * dy;
}

f32 vec2f_length(vec2f this) {
    return f32_sqrt(this.x * this.x + this.y * this.y);
}

f32 vec2f_length_squared(vec2f this) {
    return this.x * this.x + this.y * this.y;
}

// Adding, Subtracting & Multiplying

vec2f* vec2f_add(vec2f* this, vec2f vec) {
    this->x += vec.x;
    this->y += vec.y;

    return this;
}

vec2f* vec2f_subtract(vec2f* this, vec2f vec) {
    this->x -= vec.x;
    this->y -= vec.y;

    return this;
}

vec2f* vec2f_multiply(vec2f* this, vec2f vec) {
    this->x *= vec.x;
    this->y *= vec.y;

    return this;
}

vec2f* vec2f_add_xy(vec2f* this, f32 x, f32 y) {
    this->x += x;
    this->y += y;

    return this;
}

vec2f* vec2f_subtract_xy(vec2f* this, f32 x, f32 y) {
    this->x -= x;
    this->y -= y;

    return this;
}

vec2f* vec2f_multiply_xy(vec2f* this, f32 x, f32 y) {
    this->x *= x;
    this->y *= y;

    return this;
}

vec2f vec2f_add_n(vec2f this, vec2f vec) {
    return (vec2f) { this.x + vec.x, this.y + vec.y };
}

vec2f vec2f_subtract_n(vec2f this, vec2f vec) {
    return (vec2f) { this.x - vec.x, this.y - vec.y };
}

vec2f vec2f_multiply_n(vec2f this, vec2f vec) {
    return (vec2f) { this.x * vec.x, this.y * vec.y };
}

vec2f vec2f_add_n_xy(vec2f this, f32 x, f32 y) {
    return (vec2f) { this.x + x, this.y + y };
}

vec2f vec2f_subtract_n_xy(vec2f this, f32 x, f32 y) {
    return (vec2f) { this.x - x, this.y - y };
}

vec2f vec2f_multiply_n_xy(vec2f this, f32 x, f32 y) {
    return (vec2f) { this.x * x, this.y * y };
}

// Other Functions

vec2f* vec2f_relativize(vec2f* this, vec2f vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;

    return this;
}

vec2f vec2f_relativize_n(vec2f this, vec2f vec) {
    return (vec2f) { this.x - vec.x, this.y - vec.y };
}

vec2f* vec2f_normalize(vec2f* this) {
    f32 distance = f32_sqrt(this->x * this->x + this->y * this->y);

    if (distance < 1.0E-4) {
        this->x = 0;
        this->y = 0;
    } else {
        this->x = this->x / distance;
        this->y = this->y / distance;
    }

    return this;
}

vec2f vec2f_normalize_n(vec2f this) {
    f32 distance = f32_sqrt(this.x * this.x + this.y * this.y);
    return distance < 1.0E-4 ? VEC2F_ZERO : (vec2f) { this.x / distance, this.y / distance };
}

vec2f* vec2f_negate(vec2f* this) {
    this->x = -this->x;
    this->y = -this->y;

    return this;
}

vec2f vec2f_negate_n(vec2f this) {
    return (vec2f) { -this.x, -this.y };
}

bool vec2f_equals(vec2f this, vec2f vec) {
    return this.x == vec.x && this.y == vec.y;
}
