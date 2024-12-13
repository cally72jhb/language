#include "vec2i.h"

#include "common/constants.h"

#include "common/math/primitives/f64_math.h"

// Vec2i Functions

// Dot Product, Distance & Length

i32 vec2i_dot_product(vec2i this, vec2i vec) {
    return this.x * vec.x + this.y * vec.y;
}

f64 vec2i_distance(vec2i this, vec2i vec) {
    f64 dx = (f64) (this.x - vec.x);
    f64 dy = (f64) (this.y - vec.y);

    return f64_sqrt(dx * dx + dy * dy);
}

i32 vec2i_distance_squared(vec2i this, vec2i vec) {
    i32 dx = this.x - vec.x;
    i32 dy = this.y - vec.y;

    return dx * dx + dy * dy;
}

f64 vec2i_length(vec2i this) {
    return f64_sqrt((f64) (this.x * this.x + this.y * this.y));
}

i32 vec2i_length_squared(vec2i this) {
    return this.x * this.x + this.y * this.y;
}

// Adding, Subtracting & Multiplying

vec2i* vec2i_add(vec2i* this, vec2i vec) {
    this->x += vec.x;
    this->y += vec.y;

    return this;
}

vec2i* vec2i_subtract(vec2i* this, vec2i vec) {
    this->x -= vec.x;
    this->y -= vec.y;

    return this;
}

vec2i* vec2i_multiply(vec2i* this, vec2i vec) {
    this->x *= vec.x;
    this->y *= vec.y;

    return this;
}

vec2i* vec2i_add_xy(vec2i* this, i32 x, i32 y) {
    this->x += x;
    this->y += y;

    return this;
}

vec2i* vec2i_subtract_xy(vec2i* this, i32 x, i32 y) {
    this->x -= x;
    this->y -= y;

    return this;
}

vec2i* vec2i_multiply_xy(vec2i* this, i32 x, i32 y) {
    this->x *= x;
    this->y *= y;

    return this;
}

vec2i vec2i_add_n(vec2i this, vec2i vec) {
    return (vec2i) { this.x + vec.x, this.y + vec.y };
}

vec2i vec2i_subtract_n(vec2i this, vec2i vec) {
    return (vec2i) { this.x - vec.x, this.y - vec.y };
}

vec2i vec2i_multiply_n(vec2i this, vec2i vec) {
    return (vec2i) { this.x * vec.x, this.y * vec.y };
}

vec2i vec2i_add_n_xy(vec2i this, i32 x, i32 y) {
    return (vec2i) { this.x + x, this.y + y };
}

vec2i vec2i_subtract_n_xy(vec2i this, i32 x, i32 y) {
    return (vec2i) { this.x - x, this.y - y };
}

vec2i vec2i_multiply_n_xy(vec2i this, i32 x, i32 y) {
    return (vec2i) { this.x * x, this.y * y };
}

// Other Functions

vec2i* vec2i_relativize(vec2i* this, vec2i vec) {
    this->x = this->x - vec.x;
    this->y = this->y - vec.y;

    return this;
}

vec2i vec2i_relativize_n(vec2i this, vec2i vec) {
    return (vec2i) { this.x - vec.x, this.y - vec.y };
}

vec2i* vec2i_normalize(vec2i* this) {
    f64 distance = f64_sqrt((f64) (this->x * this->x + this->y * this->y));

    if (distance < 1.0E-4) {
        this->x = 0;
        this->y = 0;
    } else {
        this->x = (i32) (((f64) this->x) / distance);
        this->y = (i32) (((f64) this->y) / distance);
    }

    return this;
}

vec2i vec2i_normalize_n(vec2i this) {
    f64 distance = f64_sqrt((f64) (this.x * this.x + this.y * this.y));
    return distance < 1.0E-4 ? VEC2I_ZERO : (vec2i) { (i32) (((f64) this.x) / distance), (i32) (((f64) this.y) / distance) };
}

vec2i* vec2i_negate(vec2i* this) {
    this->x = -this->x;
    this->y = -this->y;

    return this;
}

vec2i vec2i_negate_n(vec2i this) {
    return (vec2i) { -this.x, -this.y };
}

bool vec2i_equals(vec2i this, vec2i vec) {
    return this.x == vec.x && this.y == vec.y;
}
