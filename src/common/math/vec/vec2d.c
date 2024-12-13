#include "vec2d.h"

#include "common/constants.h"

#include "common/math/primitives/f64_math.h"

// Vec2d Functions

// Dot Product, Distance & Length

f64 vec2d_dot_product(vec2d this, vec2d vec) {
    return this.x * vec.x + this.y * vec.y;
}

f64 vec2d_distance(vec2d this, vec2d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;

    return f64_sqrt(dx * dx + dy * dy);
}

f64 vec2d_distance_squared(vec2d this, vec2d vec) {
    f64 dx = this.x - vec.x;
    f64 dy = this.y - vec.y;

    return dx * dx + dy * dy;
}

f64 vec2d_length(vec2d this) {
    return f64_sqrt(this.x * this.x + this.y * this.y);
}

f64 vec2d_length_squared(vec2d this) {
    return this.x * this.x + this.y * this.y;
}

// Adding, Subtracting & Multiplying

vec2d* vec2d_add(vec2d* this, vec2d vec) {
    this->x += vec.x;
    this->y += vec.y;

    return this;
}

vec2d* vec2d_subtract(vec2d* this, vec2d vec) {
    this->x -= vec.x;
    this->y -= vec.y;

    return this;
}

vec2d* vec2d_multiply(vec2d* this, vec2d vec) {
    this->x *= vec.x;
    this->y *= vec.y;

    return this;
}

vec2d* vec2d_add_xy(vec2d* this, f64 x, f64 y) {
    this->x += x;
    this->y += y;

    return this;
}

vec2d* vec2d_subtract_xy(vec2d* this, f64 x, f64 y) {
    this->x -= x;
    this->y -= y;

    return this;
}

vec2d* vec2d_multiply_xy(vec2d* this, f64 x, f64 y) {
    this->x *= x;
    this->y *= y;

    return this;
}

vec2d vec2d_add_n(vec2d this, vec2d vec) {
    return (vec2d) { this.x + vec.x, this.y + vec.y };
}

vec2d vec2d_subtract_n(vec2d this, vec2d vec) {
    return (vec2d) { this.x - vec.x, this.y - vec.y };
}

vec2d vec2d_multiply_n(vec2d this, vec2d vec) {
    return (vec2d) { this.x * vec.x, this.y * vec.y };
}

vec2d vec2d_add_n_xy(vec2d this, f64 x, f64 y) {
    return (vec2d) { this.x + x, this.y + y };
}

vec2d vec2d_subtract_n_xy(vec2d this, f64 x, f64 y) {
    return (vec2d) { this.x - x, this.y - y };
}

vec2d vec2d_multiply_n_xy(vec2d this, f64 x, f64 y) {
    return (vec2d) { this.x * x, this.y * y };
}

// Other Functions

vec2d* vec2d_relativize(vec2d* this, vec2d vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;

    return this;
}

vec2d vec2d_relativize_n(vec2d this, vec2d vec) {
    return (vec2d) { this.x - vec.x, this.y - vec.y };
}

vec2d* vec2d_normalize(vec2d* this) {
    f64 distance = f64_sqrt(this->x * this->x + this->y * this->y);

    if (distance < 1.0E-4) {
        this->x = 0;
        this->y = 0;
    } else {
        this->x = this->x / distance;
        this->y = this->y / distance;
    }

    return this;
}

vec2d vec2d_normalize_n(vec2d this) {
    f64 distance = f64_sqrt(this.x * this.x + this.y * this.y);
    return distance < 1.0E-4 ? VEC2D_ZERO : (vec2d) { this.x / distance, this.y / distance };
}

vec2d* vec2d_negate(vec2d* this) {
    this->x = -this->x;
    this->y = -this->y;

    return this;
}

vec2d vec2d_negate_n(vec2d this) {
    return (vec2d) { -this.x, -this.y };
}

bool vec2d_equals(vec2d this, vec2d vec) {
    return this.x == vec.x && this.y == vec.y;
}
