#include "f64_math.h"

#include <math.h>

#include "common/constants.h"

// F64 Math Functions

f64 f64_abs(f64 value) {
    return value < 0.0 ? -value : value;
}

f64 f64_neg(f64 value) {
    return value > 0.0 ? -value : value;
}

f64 f64_min(f64 value1, f64 value2) {
    return value1 < value2 ? value1 : value2;
}

f64 f64_max(f64 value1, f64 value2) {
    return value1 > value2 ? value1 : value2;
}

f64 f64_max3(f64 value1, f64 value2, f64 value3) {
    return value1 > value2 ? (value1 > value3 ? value1 : value3) : (value2 > value3 ? value2 : value3);
}

f64 f64_min3(f64 value1, f64 value2, f64 value3) {
    return value1 < value2 ? (value1 < value3 ? value1 : value3) : (value2 < value3 ? value2 : value3);
}

f64 f64_clamp(f64 value, f64 min, f64 max) {
    return value < min ? min : (value > max ? max : value);
}

f64 f64_signum(f64 value) {
    return (value > 0.0) ? +1.0 : ((value < 0.0) ? -1.0 : 0.0);
}

f64 f64_lerp(f64 delta, f64 start, f64 end) {
    return start + delta * (end - start);
}

f64 f64_deg(f64 rad) {
    return rad * 180.0 / F64_PI;
}

f64 f64_rad(f64 deg) {
    return deg / 180.0 * F64_PI;
}

f64 f64_sin(f64 value) {
    return sin(value);
}

f64 f64_cos(f64 value) {
    return cos(value);
}

f64 f64_tan(f64 value) {
    return tan(value);
}

f64 f64_asin(f64 value) {
    return asin(value);
}

f64 f64_acos(f64 value) {
    return acos(value);
}

f64 f64_atan(f64 value) {
    return atan(value);
}

f64 f64_atan2(f64 x, f64 y) {
    return atan2(x, y);
}

f64 f64_ceil(f64 value) {
    return ceil(value);
}

f64 f64_floor(f64 value) {
    return floor(value);
}

f64 f64_round(f64 value) {
    return round(value);
}

f64 f64_log(f64 value) {
    return log(value);
}

f64 f64_log2(f64 value) {
    return log2(value);
}

f64 f64_log10(f64 value) {
    return log10(value);
}

f64 f64_logx(f64 base, f64 value) {
    return log(value) / log(base);
}

f64 f64_mod(f64 value1, f64 value2) {
    return fmod(value1, value2);
}

f64 f64_pow(f64 base, f64 exponent) {
    return pow(base, exponent);
}

f64 f64_pow10(f64 exponent) {
    return pow(10, exponent);
}

void f64_part(f64 value, f64* integer_part, f64* fractional_part) {
    *fractional_part = modf(value, integer_part);
}

f64 f64_sqrt(f64 value) {
    return sqrt(value);
}

f64 f64_inverse_sqrt(f64 value) {
    return 1.0 / sqrt(value);
}
