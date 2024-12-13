#include "f32_math.h"

#include <math.h>

#include "common/constants.h"

// F32 Math Functions

f32 f32_abs(f32 value) {
    return value < 0.0F ? -value : value;
}

f32 f32_neg(f32 value) {
    return value > 0.0F ? -value : value;
}

f32 f32_min(f32 value1, f32 value2) {
    return value1 < value2 ? value1 : value2;
}

f32 f32_max(f32 value1, f32 value2) {
    return value1 > value2 ? value1 : value2;
}

f32 f32_max3(f32 value1, f32 value2, f32 value3) {
    return value1 > value2 ? (value1 > value3 ? value1 : value3) : (value2 > value3 ? value2 : value3);
}

f32 f32_min3(f32 value1, f32 value2, f32 value3) {
    return value1 < value2 ? (value1 < value3 ? value1 : value3) : (value2 < value3 ? value2 : value3);
}

f32 f32_clamp(f32 value, f32 min, f32 max) {
    return value < min ? min : (value > max ? max : value);
}

f32 f32_signum(f32 value) {
    return (value > 0.0F) ? +1.0F : ((value < 0.0F) ? -1.0F : 0.0F);
}

f32 f32_lerp(f32 delta, f32 start, f32 end) {
    return start + delta * (end - start);
}

f32 f32_deg(f32 rad) {
    return rad * 180.0F / F32_PI;
}

f32 f32_rad(f32 deg) {
    return deg / 180.0F * F32_PI;
}

f32 f32_sin(f32 value) {
    return sinf(value);
}

f32 f32_cos(f32 value) {
    return cosf(value);
}

f32 f32_tan(f32 value) {
    return tanf(value);
}

f32 f32_asin(f32 value) {
    return asinf(value);
}

f32 f32_acos(f32 value) {
    return acosf(value);
}

f32 f32_atan(f32 value) {
    return atanf(value);
}

f32 f32_atan2(f32 x, f32 y) {
    return atan2f(x, y);
}

f32 f32_ceil(f32 value) {
    return ceilf(value);
}

f32 f32_floor(f32 value) {
    return floorf(value);
}

f32 f32_round(f32 value) {
    return roundf(value);
}

f32 f32_log(f32 value) {
    return logf(value);
}

f32 f32_log2(f32 value) {
    return log2f(value);
}

f32 f32_log10(f32 value) {
    return log10f(value);
}

f32 f32_logx(f32 base, f32 value) {
    return logf(value) / logf(base);
}

f32 f32_mod(f32 value1, f32 value2) {
    return fmodf(value1, value2);
}

f32 f32_pow(f32 base, f32 exponent) {
    return powf(base, exponent);
}

f32 f32_pow10(f32 exponent) {
    return powf(10, exponent);
}

void f32_part(f32 value, f32* integer_part, f32* fractional_part) {
    *fractional_part = modff(value, integer_part);
}

f32 f32_sqrt(f32 value) {
    return sqrtf(value);
}

f32 f32_inverse_sqrt(f32 value) {
    return 1.0F / sqrtf(value);
}
