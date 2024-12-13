#include "mat4.h"

#include "common/constants.h"

// Mat4 Functions

bool mat4_equals(mat4* this, mat4* mat) {
    return (
        this->m00 == mat->m00 && this->m01 == mat->m01 && this->m02 == mat->m02 && this->m03 == mat->m03 &&
        this->m10 == mat->m10 && this->m11 == mat->m11 && this->m12 == mat->m12 && this->m13 == mat->m13 &&
        this->m20 == mat->m20 && this->m21 == mat->m21 && this->m22 == mat->m22 && this->m23 == mat->m23 &&
        this->m30 == mat->m30 && this->m31 == mat->m31 && this->m32 == mat->m32 && this->m33 == mat->m33
    );
}

void mat4_scale(mat4* this, f32 scale) {
    this->m00 *= scale; this->m01 *= scale, this->m02 *= scale; this->m03 *= scale;
    this->m10 *= scale; this->m11 *= scale, this->m12 *= scale; this->m13 *= scale;
    this->m20 *= scale; this->m21 *= scale, this->m22 *= scale; this->m23 *= scale;
    this->m30 *= scale; this->m31 *= scale, this->m32 *= scale; this->m33 *= scale;
}

mat4 mat4_inverse(mat4* this) {
    mat4 result;

    f32 temp[6] = { 0 };

    f32 m00 = this->m00; f32 m01 = this->m01; f32 m02 = this->m02; f32 m03 = this->m03;
    f32 m10 = this->m10; f32 m11 = this->m11; f32 m12 = this->m12; f32 m13 = this->m13;
    f32 m20 = this->m20; f32 m21 = this->m21; f32 m22 = this->m22; f32 m23 = this->m23;
    f32 m30 = this->m30; f32 m31 = this->m31; f32 m32 = this->m32; f32 m33 = this->m33;

    temp[0] = m22 * m33 - m32 * m23;
    temp[1] = m21 * m33 - m31 * m23;
    temp[2] = m21 * m32 - m31 * m22;
    temp[3] = m20 * m33 - m30 * m23;
    temp[4] = m20 * m32 - m30 * m22;
    temp[5] = m20 * m31 - m30 * m21;

    result.m00 = m11 * temp[0] - m12 * temp[1] + m13 * temp[2];
    result.m10 =-(m10 * temp[0] - m12 * temp[3] + m13 * temp[4]);
    result.m20 =  m10 * temp[1] - m11 * temp[3] + m13 * temp[5];
    result.m30 =-(m10 * temp[2] - m11 * temp[4] + m12 * temp[5]);

    result.m01 =-(m01 * temp[0] - m02 * temp[1] + m03 * temp[2]);
    result.m11 =  m00 * temp[0] - m02 * temp[3] + m03 * temp[4];
    result.m21 =-(m00 * temp[1] - m01 * temp[3] + m03 * temp[5]);
    result.m31 =  m00 * temp[2] - m01 * temp[4] + m02 * temp[5];

    temp[0] = m12 * m33 - m32 * m13;
    temp[1] = m11 * m33 - m31 * m13;
    temp[2] = m11 * m32 - m31 * m12;
    temp[3] = m10 * m33 - m30 * m13;
    temp[4] = m10 * m32 - m30 * m12;
    temp[5] = m10 * m31 - m30 * m11;

    result.m02 =  m01 * temp[0] - m02 * temp[1] + m03 * temp[2];
    result.m12 =-(m00 * temp[0] - m02 * temp[3] + m03 * temp[4]);
    result.m22 =  m00 * temp[1] - m01 * temp[3] + m03 * temp[5];
    result.m32 =-(m00 * temp[2] - m01 * temp[4] + m02 * temp[5]);

    temp[0] = m12 * m23 - m22 * m13;
    temp[1] = m11 * m23 - m21 * m13;
    temp[2] = m11 * m22 - m21 * m12;
    temp[3] = m10 * m23 - m20 * m13;
    temp[4] = m10 * m22 - m20 * m12;
    temp[5] = m10 * m21 - m20 * m11;

    result.m03 =-(m01 * temp[0] - m02 * temp[1] + m03 * temp[2]);
    result.m13 =  m00 * temp[0] - m02 * temp[3] + m03 * temp[4];
    result.m23 =-(m00 * temp[1] - m01 * temp[3] + m03 * temp[5]);
    result.m33 =  m00 * temp[2] - m01 * temp[4] + m02 * temp[5];

    mat4_scale(&result, 1.0F / (m00 * result.m00 + m01 * result.m10 + m02 * result.m20 + m03 * result.m30));

    return result;
}
