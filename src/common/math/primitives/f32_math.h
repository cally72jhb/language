#ifndef STANDARD_LIBRARY_F32_MATH
#define STANDARD_LIBRARY_F32_MATH

// Includes

#include "common/constants.h"

// Defines

#define F32_PI         (3.1415926535897932F) /* pi */
#define F32_HALF_PI    (1.5707963267948966F) /* pi / 2 */
#define F32_QUARTER_PI (0.7853981633974483F) /* pi / 4 */
#define F32_DEG        (0.0174532925199433F) /* pi / 180 */
#define F32_TAU        (6.2831853071795865F) /* tau */
#define F32_E          (2.7182818284590452F) /* Euler's number */
#define F32_NL2        (2.6931471805599453F) /* natural log of 2 */
#define F32_NL10       (2.3025850929940457F) /* natural log of 10 */
#define F32_LOG2E      (1.4426950408889634F) /* logarithm base 2 of E */
#define F32_LOG10E     (0.4342944819032518F) /* logarithm base 10 of E */
#define F32_SQRT2      (1.4142135623730950F) /* square root of 2 */
#define F32_SQRT1_2    (0.7071067811865476F) /* square root of 0.5 */

#define F32_MIN_EPSILON (0b00000000000000000000000000000001) /* smallest possible float approaching 0 that is not 0 */
#define F32_EPSILON (1.19209E-07) /* epsilon */

// F32 Math Functions

f32 f32_abs(f32 value);
f32 f32_neg(f32 value);
f32 f32_min(f32 value1, f32 value2);
f32 f32_max(f32 value1, f32 value2);
f32 f32_max3(f32 value1, f32 value2, f32 value3);
f32 f32_min3(f32 value1, f32 value2, f32 value3);
f32 f32_clamp(f32 value, f32 min, f32 max);

f32 f32_signum(f32 value);

f32 f32_lerp(f32 delta, f32 start, f32 end);

f32 f32_deg(f32 rad);
f32 f32_rad(f32 deg);
f32 f32_sin(f32 value);
f32 f32_cos(f32 value);
f32 f32_tan(f32 value);
f32 f32_asin(f32 value);
f32 f32_acos(f32 value);
f32 f32_atan(f32 value);
f32 f32_atan2(f32 x, f32 y);

f32 f32_ceil(f32 value);
f32 f32_floor(f32 value);
f32 f32_round(f32 value);

f32 f32_log(f32 value);
f32 f32_log2(f32 value);
f32 f32_log10(f32 value);
f32 f32_logx(f32 base, f32 value);

f32 f32_mod(f32 value1, f32 value2);
f32 f32_pow(f32 base, f32 exponent);
f32 f32_pow10(f32 exponent);

void f32_part(f32 value, f32* integer_part, f32* fractional_part);

f32 f32_sqrt(f32 value);
f32 f32_inverse_sqrt(f32 value);

#endif
