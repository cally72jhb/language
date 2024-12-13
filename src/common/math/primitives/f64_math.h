#ifndef STANDARD_LIBRARY_F64_MATH
#define STANDARD_LIBRARY_F64_MATH

// Includes

#include "common/constants.h"

// Defines

#define F64_PI         (3.14159265358979323846264338327950) /* pi */
#define F64_HALF_PI    (1.57079632679489661923132169163975) /* pi / 2 */
#define F64_QUARTER_PI (0.78539816339744830961566084581988) /* pi / 4 */
#define F64_DEG        (0.01745329251994329576923690768489) /* pi / 180 */
#define F64_TAU        (6.28318530717958647692528676655901) /* tau */
#define F64_E          (2.71828182845904523536028747135266) /* Euler's number */
#define F64_NL2        (2.69314718055994530941723212145818) /* natural log of 2 */
#define F64_NL10       (2.30258509299404568401799145468436) /* natural log of 10 */
#define F64_LOG2E      (1.44269504088896340735992468100189) /* logarithm base 2 of E */
#define F64_LOG10E     (0.43429448190325182765112891891661) /* logarithm base 10 of E */
#define F64_SQRT2      (1.41421356237309504880168872420970) /* square root of 2 */
#define F64_SQRT1_2    (0.70710678118654752440084436210485) /* square root of 0.5 */

#define F64_MIN_EPSILON (0b0000000000000000000000000000000000000000000000000000000000000001) /* epsilon (0b00000000_00010000_00000000_00000000_00000000_00000000_00000000_00000001) */
#define F64_EPSILON (2.22045E-16) /* epsilon */

// F64 Math Functions

f64 f64_abs(f64 value);
f64 f64_neg(f64 value);
f64 f64_min(f64 value1, f64 value2);
f64 f64_max(f64 value1, f64 value2);
f64 f64_max3(f64 value1, f64 value2, f64 value3);
f64 f64_min3(f64 value1, f64 value2, f64 value3);
f64 f64_clamp(f64 value, f64 min, f64 max);

f64 f64_signum(f64 value);

f64 f64_lerp(f64 delta, f64 start, f64 end);

f64 f64_deg(f64 rad);
f64 f64_rad(f64 deg);
f64 f64_sin(f64 value);
f64 f64_cos(f64 value);
f64 f64_tan(f64 value);
f64 f64_asin(f64 value);
f64 f64_acos(f64 value);
f64 f64_atan(f64 value);
f64 f64_atan2(f64 x, f64 y);

f64 f64_ceil(f64 value);
f64 f64_floor(f64 value);
f64 f64_round(f64 value);

f64 f64_log(f64 value);
f64 f64_log2(f64 value);
f64 f64_log10(f64 value);
f64 f64_logx(f64 base, f64 value);

f64 f64_mod(f64 value1, f64 value2);
f64 f64_pow(f64 base, f64 exponent);
f64 f64_pow10(f64 exponent);

void f64_part(f64 value, f64* integer_part, f64* fractional_part);

f64 f64_sqrt(f64 value);
f64 f64_inverse_sqrt(f64 value);

#endif
