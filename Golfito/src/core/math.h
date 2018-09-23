#ifndef _MATH_H_
#define _MATH_H_

#if defined __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif 

#include <math.h>

#define MPI 3.141592653589793f

static float radToDeg(float rad)
{
    return rad * 180.0f / MPI;
}
static float degToRad(float deg)
{
    return deg * MPI / 180.0f;
}

struct quat
{
    float x, y, z, w;
};

struct vec2
{
    float x;
    float y;
};

struct vec3
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
    };
};

struct mat2d
{
    union
    {
        struct
        {
            float a, b, c;
            float d, tx, ty;
        };
        float data[6];
    };
};

struct mat4
{
    union
    {
        struct
        {
            float a, b, c, d;
            float e, f, g, h;
            float i, j, k, l;
            float pM, n, o, p;
        };
        float data[16];
    };
};

/* mat2d */
static struct mat2d* mat2dIdent(struct mat2d* __restrict pOut);
static struct mat2d* mat2dMul(struct mat2d* __restrict pOut, struct mat2d* __restrict pM0, struct mat2d* __restrict pM1);
static struct vec2* mat2DVec2Mul(struct vec2*  __restrict pOut, struct mat2d* __restrict pM0, struct vec2* __restrict pV1);
static struct mat2d* mat2DTranslate(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float x, float y);
static struct mat2d* mat2DScale(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float x, float y);
static struct mat2d* mat2DRotate(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float radian);

/* mat4 */
static struct mat4* mat4Ident(struct mat4* __restrict pOut);
static struct mat4* mat4Mul(struct mat4* __restrict pOut, struct mat4* __restrict pM0, struct mat4* __restrict pM1);
static struct vec3* mat4Vec3Mul(struct vec3* __restrict pOut, struct mat4* __restrict pM0, struct vec3* __restrict pV1);
static struct mat4* mat4Transpose(struct mat4* __restrict pOut, struct mat4* __restrict pM);
static struct mat4* mat4Invert(struct mat4* __restrict pOut, struct mat4* __restrict pM);
static struct mat4* mat4RotateX(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian);
static struct mat4* mat4RotateY(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian);
static struct mat4* mat4RotateZ(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian);
static struct mat4* mat4Rotate(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict pAxis, float radian);
static struct mat4* mat4Scale(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict pScale);
static struct mat4* mat4Translate(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict pTranslate);
static struct mat4* mat4Frustum(struct mat4* __restrict pOut, float left, float right, float bottom, float top, float frustumNear, float frustumFar);
static struct mat4* mat4Perspective(struct mat4* __restrict pOut, float fovY, float aspect, float perspNear, float perspFar);
static struct mat4* mat4Orthographic(struct mat4* __restrict pOut, float left, float right, float bottom, float top, float orthoNear, float orthoFar);
static struct mat4* mat4LookAt(struct mat4* __restrict pOut, struct vec3* __restrict pEye, struct vec3* __restrict pCenter, struct vec3* __restrict pUp);

/* vec2 */
static struct vec2* vec2Add(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static struct vec2* vec2Sub(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static struct vec2* vec2Mul(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static struct vec2* vec2Invert(struct vec2* __restrict pOut, struct vec2* __restrict pV);
static struct vec2* vec2Negate(struct vec2* __restrict pOut, struct vec2* __restrict pV);
static struct vec2* vec2Normalize(struct vec2* __restrict pOut, struct vec2* __restrict pV);
static int32_t vec2Equal(struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static struct vec3* vec2Cross(struct vec3* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static float vec2Distance(struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static float vec2Dot(struct vec2* __restrict pV0, struct vec2* __restrict pV1);
static float vec2Length(struct vec2* __restrict pV);
static float vec2Length2(struct vec2* __restrict pV);
static float vec2Distance2(struct vec2* __restrict pV0, struct vec2* __restrict pV1);

/* vec3 */
static struct vec3* vec3Add(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static struct vec3* vec3Sub(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static struct vec3* vec3Mul(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static struct vec3* vec3Invert(struct vec3* __restrict pOut, struct vec3* __restrict pV);
static struct vec3* vec3Negate(struct vec3* __restrict pOut, struct vec3* __restrict pV);
static struct vec3* vec3Normalize(struct vec3* __restrict pOut, struct vec3* __restrict pV);
static int32_t vec3Equal(struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static struct vec3* vec3Cross(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static float vec3Distance(struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static float vec3Dot(struct vec3* __restrict pV0, struct vec3* __restrict pV1);
static float vec3Length(struct vec3* __restrict pV);
static float vec3Length2(struct vec3* __restrict pV);
static float vec3Distance2(struct vec3* __restrict pV0, struct vec3* __restrict pV1);

/* quat */
static struct quat* quatIdent(struct quat* __restrict pOut);
static struct quat* quatAdd(struct quat* __restrict pOut, struct quat* __restrict pQ0, struct quat* __restrict pQ1);
static struct quat* quatMul(struct quat* __restrict pOut, struct quat* __restrict pQ0, struct quat* __restrict pQ1);
static struct quat* quatSetAxisAngle(struct quat* __restrict pOut, struct vec3* __restrict pAxis, float radian);
static struct quat* quatRotateX(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian);
static struct quat* quatRotateY(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian);
static struct quat* quatRotateZ(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian);
static struct quat* quatCalcW(struct quat* __restrict pOut, struct quat* __restrict pQ);
static struct quat* quatInvert(struct quat* __restrict pOut, struct quat* __restrict pQ);
static struct quat* quatConjugate(struct quat* __restrict pOut, struct quat* __restrict pQ);
static struct mat4* quatToMat4(struct mat4* __restrict pOut, struct quat* __restrict pQ);
static float quatGetAxisAngle(struct vec3* __restrict pOutAxis, struct quat* __restrict pQ);

/* Definition */
typedef struct quat quat_t;
typedef struct vec2 vec2_t;
typedef struct vec3 vec3_t;
typedef struct mat2d mat2d_t;
typedef struct mat4 mat4_t;

/* mat2d */
struct mat2d* mat2dIdent(struct mat2d* __restrict pOut)
{
    pOut->data[0] = 1.0f;
    pOut->data[1] = 0.0f;
    pOut->data[2] = 0.0f;
    pOut->data[3] = 1.0f;
    pOut->data[4] = 0.0f;
    pOut->data[5] = 0.0f;
    return pOut;
}
struct mat2d* mat2dMul(struct mat2d* __restrict pOut, struct mat2d* __restrict pM0, struct mat2d* __restrict pM1)
{
    const float* matrixA = pM0->data;
    const float* matrixB = pM1->data;
    float* matrix = pOut->data;
    
    float a = matrixA[0];
    float b = matrixA[1];
    float c = matrixA[2];
    float d = matrixA[3];
    float tx = matrixA[4];
    float ty = matrixA[5];
    
    float a0 = matrixB[0];
    float b0 = matrixB[1];
    float c0 = matrixB[2];
    float d0 = matrixB[3];
    float tx0 = matrixB[4];
    float ty0 = matrixB[5];
    
    matrix[0] = a * a0 + b * c0;
    matrix[1] = a * b0 + b * d0;
    matrix[2] = c * a0 + d * c0;
    matrix[3] = c * b0 + d * d0;
    matrix[4] = tx * a0 + ty * c0 + tx0;
    matrix[5] = tx * b0 + ty * d0 + ty0;
    
    return pOut;
}
struct vec2* mat2DVec2Mul(struct vec2* __restrict pOut, struct mat2d* __restrict pM0, struct vec2* __restrict pV1)
{
    pOut->x = pV1->x * pM0->a + pV1->y * pM0->c + pM0->tx;
    pOut->y = pV1->x * pM0->b + pV1->y * pM0->d + pM0->ty;
    return pOut;
}
struct mat2d* mat2DTranslate(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float x, float y)
{
    pOut->data[4] = pM->data[0] * x + pM->data[2] * y + pM->data[4];
    pOut->data[5] = pM->data[1] * x + pM->data[3] * y + pM->data[5];
    return pOut;
}
struct mat2d* mat2DScale(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float x, float y)
{
    pOut->data[0] = pM->data[0] * x;
    pOut->data[1] = pM->data[1] * x;
    pOut->data[2] = pM->data[2] * y;
    pOut->data[3] = pM->data[3] * y;
    return pOut;
}
struct mat2d* mat2DRotate(struct mat2d* __restrict pOut, struct mat2d* __restrict pM, float radian)
{
    float sn = sinf(radian);
    float cs = cosf(radian);
    pOut->data[0] = cs * pM->data[0] + sn * pM->data[2];
    pOut->data[1] = cs * pM->data[1] + sn * pM->data[3];
    pOut->data[2] = -sn * pM->data[0] + cs * pM->data[2];
    pOut->data[3] = -sn * pM->data[1] + cs * pM->data[3];
    return pOut;
}

/* mat4 */
struct mat4* mat4Ident(struct mat4* __restrict pOut)
{
    pOut->data[0] = 1.0f;
    pOut->data[1] = 0.0f;
    pOut->data[2] = 0.0f;
    pOut->data[3] = 0.0f;
    pOut->data[4] = 0.0f;
    pOut->data[5] = 1.0f;
    pOut->data[6] = 0.0f;
    pOut->data[7] = 0.0f;
    pOut->data[8] = 0.0f;
    pOut->data[9] = 0.0f;
    pOut->data[10] = 1.0f;
    pOut->data[11] = 0.0f;
    pOut->data[12] = 0.0f;
    pOut->data[13] = 0.0f;
    pOut->data[14] = 0.0f;
    pOut->data[15] = 1.0f;
    return pOut;
}
struct mat4* mat4Mul(struct mat4* __restrict pOut, struct mat4* __restrict pM0, struct mat4* __restrict pM1)
{
    pOut->data[0] = pM1->data[0] * pM0->data[0] + pM1->data[1] * pM0->data[4] + pM1->data[2] * pM0->data[8] + pM1->data[3] * pM0->data[12];
    pOut->data[1] = pM1->data[0] * pM0->data[1] + pM1->data[1] * pM0->data[5] + pM1->data[2] * pM0->data[9] + pM1->data[3] * pM0->data[13];
    pOut->data[2] = pM1->data[0] * pM0->data[2] + pM1->data[1] * pM0->data[6] + pM1->data[2] * pM0->data[10] + pM1->data[3] * pM0->data[14];
    pOut->data[3] = pM1->data[0] * pM0->data[3] + pM1->data[1] * pM0->data[7] + pM1->data[2] * pM0->data[11] + pM1->data[3] * pM0->data[15];
    pOut->data[4] = pM1->data[4] * pM0->data[0] + pM1->data[5] * pM0->data[4] + pM1->data[6] * pM0->data[8] + pM1->data[7] * pM0->data[12];
    pOut->data[5] = pM1->data[4] * pM0->data[1] + pM1->data[5] * pM0->data[5] + pM1->data[6] * pM0->data[9] + pM1->data[7] * pM0->data[13];
    pOut->data[6] = pM1->data[4] * pM0->data[2] + pM1->data[5] * pM0->data[6] + pM1->data[6] * pM0->data[10] + pM1->data[7] * pM0->data[14];
    pOut->data[7] = pM1->data[4] * pM0->data[3] + pM1->data[5] * pM0->data[7] + pM1->data[6] * pM0->data[11] + pM1->data[7] * pM0->data[15];
    pOut->data[8] = pM1->data[8] * pM0->data[0] + pM1->data[9] * pM0->data[4] + pM1->data[10] * pM0->data[8] + pM1->data[11] * pM0->data[12];
    pOut->data[9] = pM1->data[8] * pM0->data[1] + pM1->data[9] * pM0->data[5] + pM1->data[10] * pM0->data[9] + pM1->data[11] * pM0->data[13];
    pOut->data[10] = pM1->data[8] * pM0->data[2] + pM1->data[9] * pM0->data[6] + pM1->data[10] * pM0->data[10] + pM1->data[11] * pM0->data[14];
    pOut->data[11] = pM1->data[8] * pM0->data[3] + pM1->data[9] * pM0->data[7] + pM1->data[10] * pM0->data[11] + pM1->data[11] * pM0->data[15];
    pOut->data[12] = pM1->data[12] * pM0->data[0] + pM1->data[13] * pM0->data[4] + pM1->data[14] * pM0->data[8] + pM1->data[15] * pM0->data[12];
    pOut->data[13] = pM1->data[12] * pM0->data[1] + pM1->data[13] * pM0->data[5] + pM1->data[14] * pM0->data[9] + pM1->data[15] * pM0->data[13];
    pOut->data[14] = pM1->data[12] * pM0->data[2] + pM1->data[13] * pM0->data[6] + pM1->data[14] * pM0->data[10] + pM1->data[15] * pM0->data[14];
    pOut->data[15] = pM1->data[12] * pM0->data[3] + pM1->data[13] * pM0->data[7] + pM1->data[14] * pM0->data[11] + pM1->data[15] * pM0->data[15];
    return pOut;
}
struct vec3* mat4Vec3Mul(struct vec3* __restrict pOut, struct mat4* __restrict pM0, struct vec3* __restrict pV1)
{
    pOut->x = pM0->data[0] * pV1->x + pM0->data[4] * pV1->y + pM0->data[8] * pV1->z + pM0->data[12];
    pOut->y = pM0->data[1] * pV1->x + pM0->data[5] * pV1->y + pM0->data[9] * pV1->z + pM0->data[13];
    pOut->z = pM0->data[2] * pV1->x + pM0->data[6] * pV1->y + pM0->data[10] * pV1->z + pM0->data[14];
    return pOut;
}
struct mat4* mat4Transpose(struct mat4* __restrict pOut, struct mat4* __restrict pM)
{
    pOut->data[0] = pM->data[0];
    pOut->data[1] = pM->data[4];
    pOut->data[2] = pM->data[8];
    pOut->data[3] = pM->data[12];
    pOut->data[4] = pM->data[1];
    pOut->data[5] = pM->data[5];
    pOut->data[6] = pM->data[9];
    pOut->data[7] = pM->data[13];
    pOut->data[8] = pM->data[2];
    pOut->data[9] = pM->data[6];
    pOut->data[10] = pM->data[10];
    pOut->data[11] = pM->data[14];
    pOut->data[12] = pM->data[3];
    pOut->data[13] = pM->data[7];
    pOut->data[14] = pM->data[11];
    pOut->data[15] = pM->data[15];
    return pOut;
}
struct mat4* mat4Invert(struct mat4* __restrict pOut, struct mat4* __restrict pM)
{
    float d0 = pM->data[0] * pM->data[5] - pM->data[1] * pM->data[4];
    float d1 = pM->data[0] * pM->data[6] - pM->data[2] * pM->data[4];
    float d2 = pM->data[0] * pM->data[7] - pM->data[3] * pM->data[4];
    float d3 = pM->data[1] * pM->data[6] - pM->data[2] * pM->data[5];
    float d4 = pM->data[1] * pM->data[7] - pM->data[3] * pM->data[5];
    float d5 = pM->data[2] * pM->data[7] - pM->data[3] * pM->data[6];
    float d6 = pM->data[8] * pM->data[13] - pM->data[9] * pM->data[12];
    float d7 = pM->data[8] * pM->data[14] - pM->data[10] * pM->data[12];
    float d8 = pM->data[8] * pM->data[15] - pM->data[11] * pM->data[12];
    float d9 = pM->data[9] * pM->data[14] - pM->data[10] * pM->data[13];
    float d10 = pM->data[9] * pM->data[15] - pM->data[11] * pM->data[13];
    float d11 = pM->data[10] * pM->data[15] - pM->data[11] * pM->data[14];
    float determinant = d0 * d11 - d1 * d10 + d2 * d9 + d3 * d8 - d4 * d7 + d5 * d6;
    
    if (determinant == 0.0f) return pM;
    
    determinant = 1.0f / determinant;
    pOut->data[0] = (pM->data[5] * d11 - pM->data[6] * d10 + pM->data[7] * d9) * determinant;
    pOut->data[1] = (pM->data[2] * d10 - pM->data[1] * d11 - pM->data[3] * d9) * determinant;
    pOut->data[2] = (pM->data[13] * d5 - pM->data[14] * d4 + pM->data[15] * d3) * determinant;
    pOut->data[3] = (pM->data[10] * d4 - pM->data[9] * d5 - pM->data[11] * d3) * determinant;
    pOut->data[4] = (pM->data[6] * d8 - pM->data[4] * d11 - pM->data[7] * d7) * determinant;
    pOut->data[5] = (pM->data[0] * d11 - pM->data[2] * d8 + pM->data[3] * d7) * determinant;
    pOut->data[6] = (pM->data[14] * d2 - pM->data[12] * d5 - pM->data[15] * d1) * determinant;
    pOut->data[7] = (pM->data[8] * d5 - pM->data[10] * d2 + pM->data[11] * d1) * determinant;
    pOut->data[8] = (pM->data[4] * d10 - pM->data[5] * d8 + pM->data[7] * d6) * determinant;
    pOut->data[9] = (pM->data[1] * d8 - pM->data[0] * d10 - pM->data[3] * d6) * determinant;
    pOut->data[10] = (pM->data[12] * d4 - pM->data[13] * d2 + pM->data[15] * d0) * determinant;
    pOut->data[11] = (pM->data[9] * d2 - pM->data[8] * d4 - pM->data[11] * d0) * determinant;
    pOut->data[12] = (pM->data[5] * d7 - pM->data[4] * d9 - pM->data[6] * d6) * determinant;
    pOut->data[13] = (pM->data[0] * d9 - pM->data[1] * d7 + pM->data[2] * d6) * determinant;
    pOut->data[14] = (pM->data[13] * d1 - pM->data[12] * d3 - pM->data[14] * d0) * determinant;
    pOut->data[15] = (pM->data[8] * d3 - pM->data[9] * d1 + pM->data[10] * d0) * determinant;
    return pOut;
}
struct mat4* mat4RotateX(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian)
{
    float sn = sinf(radian);
    float cn = cosf(radian);
    pOut->data[0] = pM->data[0];
    pOut->data[1] = pM->data[1];
    pOut->data[2] = pM->data[2];
    pOut->data[3] = pM->data[3];
    pOut->data[12] = pM->data[12];
    pOut->data[13] = pM->data[13];
    pOut->data[14] = pM->data[14];
    pOut->data[15] = pM->data[15];
    pOut->data[4] = pM->data[4] * cn + pM->data[8] * sn;
    pOut->data[5] = pM->data[5] * cn + pM->data[9] * sn;
    pOut->data[6] = pM->data[6] * cn + pM->data[10] * sn;
    pOut->data[7] = pM->data[7] * cn + pM->data[11] * sn;
    pOut->data[8] = pM->data[8] * cn - pM->data[4] * sn;
    pOut->data[9] = pM->data[9] * cn - pM->data[5] * sn;
    pOut->data[10] = pM->data[10] * cn - pM->data[6] * sn;
    pOut->data[11] = pM->data[11] * cn - pM->data[7] * sn;
    return pOut;
}
struct mat4* mat4RotateY(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian)
{
    float sn = sinf(radian);
    float cn = cosf(radian);
    pOut->data[0] = pM->data[0] * cn - pM->data[8] * sn;
    pOut->data[1] = pM->data[1] * cn - pM->data[9] * sn;
    pOut->data[2] = pM->data[2] * cn - pM->data[10] * sn;
    pOut->data[3] = pM->data[3] * cn - pM->data[11] * sn;
    pOut->data[4] = pM->data[4];
    pOut->data[5] = pM->data[5];
    pOut->data[6] = pM->data[6];
    pOut->data[7] = pM->data[7];
    pOut->data[8] = pM->data[0] * sn + pM->data[8] * cn;
    pOut->data[9] = pM->data[1] * sn + pM->data[9] * cn;
    pOut->data[10] = pM->data[2] * sn + pM->data[10] * cn;
    pOut->data[11] = pM->data[3] * sn + pM->data[11] * cn;
    pOut->data[12] = pM->data[12];
    pOut->data[13] = pM->data[13];
    pOut->data[14] = pM->data[14];
    pOut->data[15] = pM->data[15];
    return pOut;
}
struct mat4* mat4RotateZ(struct mat4* __restrict pOut, struct mat4* __restrict pM, float radian)
{
    float sn = sinf(radian);
    float cn = cosf(radian);
    pOut->data[0] = pM->data[0] * cn + pM->data[4] * sn;
    pOut->data[1] = pM->data[1] * cn + pM->data[5] * sn;
    pOut->data[2] = pM->data[2] * cn + pM->data[6] * sn;
    pOut->data[3] = pM->data[3] * cn + pM->data[7] * sn;
    pOut->data[4] = pM->data[4] * cn - pM->data[0] * sn;
    pOut->data[5] = pM->data[5] * cn - pM->data[1] * sn;
    pOut->data[6] = pM->data[6] * cn - pM->data[2] * sn;
    pOut->data[7] = pM->data[7] * cn - pM->data[3] * sn;
    pOut->data[8] = pM->data[8];
    pOut->data[9] = pM->data[9];
    pOut->data[10] = pM->data[10];
    pOut->data[11] = pM->data[11];
    pOut->data[12] = pM->data[12];
    pOut->data[13] = pM->data[13];
    pOut->data[14] = pM->data[14];
    pOut->data[15] = pM->data[15];
    return pOut;
}
struct mat4* mat4Rotate(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict axis, float radian)
{
    float length = vec3Length(axis);
    
    if (fabsf(length) < 0.0000001f) return NULL;
    
    length = 1 / length;
    axis->x *= length;
    axis->y *= length;
    axis->z *= length;
    float sn = sinf(radian);
    float cn = cosf(radian);
    float theta = 1 - cn;
    float d0 = axis->x * axis->x * theta + cn;
    float d4 = axis->y * axis->y * theta + cn;
    float d8 = axis->z * axis->z * theta + cn;
    float d1 = axis->y * axis->x * theta + axis->z * sn;
    float d2 = axis->z * axis->x * theta - axis->y * sn;
    float d3 = axis->x * axis->y * theta - axis->z * sn;
    float d5 = axis->z * axis->y * theta + axis->x * sn;
    float d6 = axis->x * axis->z * theta + axis->y * sn;
    float d7 = axis->y * axis->z * theta - axis->x * sn;
    pOut->data[0] = pM->data[0] * d0 + pM->data[4] * d1 + pM->data[8] * d2;
    pOut->data[1] = pM->data[1] * d0 + pM->data[5] * d1 + pM->data[9] * d2;
    pOut->data[2] = pM->data[2] * d0 + pM->data[6] * d1 + pM->data[10] * d2;
    pOut->data[3] = pM->data[3] * d0 + pM->data[7] * d1 + pM->data[11] * d2;
    pOut->data[4] = pM->data[0] * d3 + pM->data[4] * d4 + pM->data[8] * d5;
    pOut->data[5] = pM->data[1] * d3 + pM->data[5] * d4 + pM->data[9] * d5;
    pOut->data[6] = pM->data[2] * d3 + pM->data[6] * d4 + pM->data[10] * d5;
    pOut->data[7] = pM->data[3] * d3 + pM->data[7] * d4 + pM->data[11] * d5;
    pOut->data[8] = pM->data[0] * d6 + pM->data[4] * d7 + pM->data[8] * d8;
    pOut->data[9] = pM->data[1] * d6 + pM->data[5] * d7 + pM->data[9] * d8;
    pOut->data[10] = pM->data[2] * d6 + pM->data[6] * d7 + pM->data[10] * d8;
    pOut->data[11] = pM->data[3] * d6 + pM->data[7] * d7 + pM->data[11] * d8;
    pOut->data[12] = pM->data[12];
    pOut->data[13] = pM->data[13];
    pOut->data[14] = pM->data[14];
    pOut->data[15] = pM->data[15];
    return pOut;
}
struct mat4* mat4Scale(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict scale)
{
    pOut->data[0] = pM->data[0] * scale->x;
    pOut->data[1] = pM->data[1] * scale->x;
    pOut->data[2] = pM->data[2] * scale->x;
    pOut->data[3] = pM->data[3] * scale->x;
    pOut->data[4] = pM->data[4] * scale->y;
    pOut->data[5] = pM->data[5] * scale->y;
    pOut->data[6] = pM->data[6] * scale->y;
    pOut->data[7] = pM->data[7] * scale->y;
    pOut->data[8] = pM->data[8] * scale->z;
    pOut->data[9] = pM->data[9] * scale->z;
    pOut->data[10] = pM->data[10] * scale->z;
    pOut->data[11] = pM->data[11] * scale->z;
    pOut->data[12] = pM->data[12];
    pOut->data[13] = pM->data[13];
    pOut->data[14] = pM->data[14];
    pOut->data[15] = pM->data[15];
    return pOut;
}
struct mat4* mat4Translate(struct mat4* __restrict pOut, struct mat4* __restrict pM, struct vec3* __restrict translate)
{
    pOut->data[0] = pM->data[0];
    pOut->data[1] = pM->data[1];
    pOut->data[2] = pM->data[2];
    pOut->data[3] = pM->data[3];
    pOut->data[4] = pM->data[4];
    pOut->data[5] = pM->data[5];
    pOut->data[6] = pM->data[6];
    pOut->data[7] = pM->data[7];
    pOut->data[8] = pM->data[8];
    pOut->data[9] = pM->data[9];
    pOut->data[10] = pM->data[10];
    pOut->data[11] = pM->data[11];
    pOut->data[12] = pM->data[0] * translate->x + pM->data[4] * translate->y + pM->data[8] * translate->z + pM->data[12];
    pOut->data[13] = pM->data[1] * translate->x + pM->data[5] * translate->y + pM->data[9] * translate->z + pM->data[13];
    pOut->data[14] = pM->data[2] * translate->x + pM->data[6] * translate->y + pM->data[10] * translate->z + pM->data[14];
    pOut->data[15] = pM->data[3] * translate->x + pM->data[7] * translate->y + pM->data[11] * translate->z + pM->data[15];
    return pOut;
}
struct mat4* mat4Frustum(struct mat4* __restrict pOut, float left, float right, float bottom, float top, float frustumNear, float frustumFar)
{
    float rightLeft = 1.0f / (right - left);
    float topBottom = 1.0f / (top - bottom);
    float nearFar = 1.0f / (frustumNear - frustumFar);
    pOut->data[0] = (frustumNear * 2.0f) * rightLeft;
    pOut->data[1] = 0.0f;
    pOut->data[2] = 0.0f;
    pOut->data[3] = 0.0f;
    pOut->data[4] = 0.0f;
    pOut->data[5] = (frustumNear * 2.0f) * topBottom;
    pOut->data[6] = 0.0f;
    pOut->data[7] = 0.0f;
    pOut->data[8] = (right + left) * rightLeft;
    pOut->data[9] = (top + bottom) * topBottom;
    pOut->data[10] = (frustumFar + frustumNear) * nearFar;
    pOut->data[11] = -1.0f;
    pOut->data[12] = 0.0f;
    pOut->data[13] = 0.0f;
    pOut->data[14] = (frustumFar * frustumNear * 2.0f) * nearFar;
    pOut->data[15] = 0.0f;
    return pOut;
}
struct mat4* mat4Perspective(struct mat4* __restrict pOut, float fovY, float aspect, float perspNear, float perspFar)
{
    float fov = 1.0f / tanf(fovY / 2.0f);
    float nearFar = 1.0f / (perspNear - perspFar);
    pOut->data[0] = fov / aspect;
    pOut->data[1] = 0.0f;
    pOut->data[2] = 0.0f;
    pOut->data[3] = 0.0f;
    pOut->data[4] = 0.0f;
    pOut->data[5] = fov;
    pOut->data[6] = 0.0f;
    pOut->data[7] = 0.0f;
    pOut->data[8] = 0.0f;
    pOut->data[9] = 0.0f;
    pOut->data[10] = (perspFar + perspNear) * nearFar;
    pOut->data[11] = -1.0f;
    pOut->data[12] = 0.0f;
    pOut->data[13] = 0.0f;
    pOut->data[14] = (2.0f * perspFar * perspNear) * nearFar;
    pOut->data[15] = 0.0f;
    return pOut;
}
struct mat4* mat4Orthographic(struct mat4* __restrict pOut, float left, float right, float bottom, float top, float orthoNear, float orthoFar)
{
    float leftRight = 1.0f / (left - right);
    float bottomTop = 1.0f / (bottom - top);
    float nearFar = 1.0f / (orthoNear - orthoFar);
    pOut->data[0] = -2.0f * leftRight;
    pOut->data[1] = 0.0f;
    pOut->data[2] = 0.0f;
    pOut->data[3] = 0.0f;
    pOut->data[4] = 0.0f;
    pOut->data[5] = -2.0f * bottomTop;
    pOut->data[6] = 0.0f;
    pOut->data[7] = 0.0f;
    pOut->data[8] = 0.0f;
    pOut->data[9] = 0.0f;
    pOut->data[10] = 2.0f * nearFar;
    pOut->data[11] = 0.0f;
    pOut->data[12] = (left + right) * leftRight;
    pOut->data[13] = (top + bottom) * bottomTop;
    pOut->data[14] = (orthoFar + orthoNear) * nearFar;
    pOut->data[15] = 1.0f;
    return pOut;
}
struct mat4* mat4LookAt(struct mat4* __restrict pOut, struct vec3* __restrict pEye, struct vec3* __restrict pCenter, struct vec3* __restrict pUp)
{
    if (fabsf(pEye->x - pCenter->x) < 0.0000001f &&
        fabsf(pEye->y - pCenter->y) < 0.0000001f &&
        fabsf(pEye->z - pCenter->z) < 0.0000001f )
    {
        return mat4Ident(pOut);
    }
    float z0 = pEye->x - pCenter->x;
    float z1 = pEye->y - pCenter->y;
    float z2 = pEye->z - pCenter->z;
    float len = 1.0f / sqrtf(z0 * z0 + z1 * z1 + z2 * z2);
    z0 *= len;
    z1 *= len;
    z2 *= len;
    float x0 = pUp->y * z2 - pUp->z * z1;
    float x1 = pUp->z * z0 - pUp->x * z2;
    float x2 = pUp->x * z1 - pUp->y * z0;
    len = sqrtf(x0 * x0 + x1 * x1 + x2 * x2);
    if (!len)
    {
        x0 = 0.0f;
        x1 = 0.0f;
        x2 = 0.0f;
    }
    else
    {
        len = 1.0f / len;
        x0 *= len;
        x1 *= len;
        x2 *= len;
    }
    float y0 = z1 * x2 - z2 * x1;
    float y1 = z2 * x0 - z0 * x2;
    float y2 = z0 * x1 - z1 * x0;
    len = sqrtf(y0 * y0 + y1 * y1 + y2 * y2);
    if (!len)
    {
        y0 = 0.0f;
        y1 = 0.0f;
        y2 = 0.0f;
    }
    else
    {
        len = 1.0f / len;
        y0 *= len;
        y1 *= len;
        y2 *= len;
    }
    pOut->data[0] = x0;
    pOut->data[1] = y0;
    pOut->data[2] = z0;
    pOut->data[3] = 0.0f;
    pOut->data[4] = x1;
    pOut->data[5] = y1;
    pOut->data[6] = z1;
    pOut->data[7] = 0.0f;
    pOut->data[8] = x2;
    pOut->data[9] = y2;
    pOut->data[10] = z2;
    pOut->data[11] = 0.0f;
    pOut->data[12] = -(x0 * pEye->x + x1 * pEye->y + x2 * pEye->z);
    pOut->data[13] = -(y0 * pEye->x + y1 * pEye->y + y2 * pEye->z);
    pOut->data[14] = -(z0 * pEye->x + z1 * pEye->y + z2 * pEye->z);
    pOut->data[15] = 1.0f;
    return pOut;
}

/* vec2 */
struct vec2* vec2Add(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    pOut->x = pV0->x + pV1->x;
    pOut->y = pV0->y + pV1->y;
    return pOut;
}
struct vec2* vec2Sub(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    pOut->x = pV0->x - pV1->x;
    pOut->y = pV0->y - pV1->y;
    return pOut;
}
struct vec2* vec2Mul(struct vec2* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    pOut->x = pV0->x * pV1->x;
    pOut->y = pV0->y * pV1->y;
    return pOut;
}
struct vec2* vec2Invert(struct vec2* __restrict pOut, struct vec2* __restrict pV)
{
    pOut->x = 1.0f / pV->x;
    pOut->y = 1.0f / pV->y;
    return pOut;
}
struct vec2* vec2Negate(struct vec2* __restrict pOut, struct vec2* __restrict pV)
{
    pOut->x = pV->x * -1.0f;
    pOut->y = pV->y * -1.0f;
    return pOut;
}
struct vec2* vec2Normalize(struct vec2* __restrict pOut, struct vec2* __restrict pV)
{
    float l = pV->x * pV->x + pV->y * pV->y;
    if (l > 0.0f)
    {
        l = 1.0f / sqrtf(l);
        pOut->x = pV->x * l;
        pOut->y = pV->y * l;
    }
    return pOut;
}
int32_t vec2Equal(struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    return (pV0->x == pV1->x && pV0->y == pV1->y);
}
struct vec3* vec2Cross(struct vec3* __restrict pOut, struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    pOut->x = 0;
    pOut->y = 0;
    pOut->z = pV0->x * pV1->y - pV0->y * pV1->x;
    return pOut;
}
float vec2Distance(struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    float x = pV0->x - pV1->x;
    float y = pV0->y - pV1->y;
    return sqrtf(x * x + y * y);
}
float vec2Dot(struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    return pV0->x * pV1->x + pV0->y * pV1->y;
}
float vec2Length(struct vec2* __restrict pV)
{
    return sqrtf(pV->x * pV->x + pV->y * pV->y);
}
float vec2Length2(struct vec2* __restrict pV)
{
    return (pV->x * pV->x + pV->y * pV->y);
}
float vec2Distance2(struct vec2* __restrict pV0, struct vec2* __restrict pV1)
{
    float x = pV0->x - pV1->x;
    float y = pV0->y - pV1->y;
    return (x * x + y * y);
}

/* vec3 */
struct vec3* vec3Add(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    pOut->x = pV0->x + pV1->x;
    pOut->y = pV0->y + pV1->y;
    pOut->z = pV0->z + pV1->z;
    return pOut;
}
struct vec3* vec3Sub(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    pOut->x = pV0->x - pV1->x;
    pOut->y = pV0->y - pV1->y;
    pOut->z = pV0->z - pV1->z;
    return pOut;
}
struct vec3* vec3Mul(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    pOut->x = pV0->x * pV1->x;
    pOut->y = pV0->y * pV1->y;
    pOut->z = pV0->z * pV1->z;
    return pOut;
}
struct vec3* vec3Invert(struct vec3* __restrict pOut, struct vec3* __restrict pV)
{
    pOut->x = 1.0f / pV->x;
    pOut->y = 1.0f / pV->y;
    pOut->z = 1.0f / pV->z;
    return pOut;
}
struct vec3* vec3Negate(struct vec3* __restrict pOut, struct vec3* __restrict pV)
{
    pOut->x = -1.0f * pV->x;
    pOut->y = -1.0f * pV->y;
    pOut->z = -1.0f * pV->z;
    return pOut;
}
struct vec3* vec3Normalize(struct vec3* __restrict pOut, struct vec3* __restrict pV)
{
    float x = pV->x;
    float y = pV->y;
    float z = pV->z;
    float l = x * x + y * y + z * z;
    if (l > 0.0f)
    {
        l = 1.0f / sqrtf(l);
        pOut->x = x * l;
        pOut->y = y * l;
        pOut->z = z * l;
    }
    return pOut;
}
int32_t vec3Equal(struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    return (pV0->x == pV1->x && pV0->y == pV1->y && pV0->z == pV1->z);
}
struct vec3* vec3Cross(struct vec3* __restrict pOut, struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    float lx = pV0->x;
    float ly = pV0->y;
    float lz = pV0->z;
    float rx = pV1->x;
    float ry = pV1->y;
    float rz = pV1->z;
    
    pOut->x = ly * rz - lz * ry;
    pOut->y = lz * rx - lx * rz;
    pOut->z = lx * ry - ly * rx;
    
    return pOut;
}
float vec3Distance(struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    float x = pV0->x - pV1->x;
    float y = pV0->y - pV1->y;
    float z = pV0->z - pV1->z;
    
    return sqrtf(x * x + y * y + z * z);
}
float vec3Dot(struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    return pV0->x * pV1->x + pV0->y * pV1->y + pV0->z * pV1->z;
}
float vec3Length(struct vec3* __restrict pV)
{
    float x = pV->x;
    float y = pV->y;
    float z = pV->z;
    return sqrtf(x * x + y * y + z * z);
}
float vec3Length2(struct vec3* __restrict pV)
{
    float x = pV->x;
    float y = pV->y;
    float z = pV->z;
    return (x * x + y * y + z * z);
}
float vec3Distance2(struct vec3* __restrict pV0, struct vec3* __restrict pV1)
{
    float x = pV0->x - pV1->x;
    float y = pV0->y - pV1->y;
    float z = pV0->z - pV1->z;
    
    return (x * x + y * y + z * z);
}

/* quat */
struct quat* quatIdent(struct quat* __restrict pOut)
{
    pOut->x = 0;
    pOut->y = 0;
    pOut->z = 0;
    pOut->w = 1;
    return pOut;
}
struct quat* quatAdd(struct quat* __restrict pOut, struct quat* __restrict pQ0, struct quat* __restrict pQ1)
{
    pOut->w = pQ0->w + pQ1->w;
    pOut->x = pQ0->x + pQ1->x;
    pOut->y = pQ0->y + pQ1->y;
    pOut->z = pQ0->z + pQ1->z;
    return pOut;
}
struct quat* quatMul(struct quat* __restrict pOut, struct quat* __restrict pQ0, struct quat* __restrict pQ1)
{
    pOut->x = pQ0->x * pQ1->w + pQ0->w * pQ1->x + pQ0->y * pQ1->z - pQ0->z * pQ1->y;
    pOut->y = pQ0->y * pQ1->w + pQ0->w * pQ1->y + pQ0->z * pQ1->x - pQ0->x * pQ1->z;
    pOut->z = pQ0->z * pQ1->w + pQ0->w * pQ1->z + pQ0->x * pQ1->y - pQ0->y * pQ1->x;
    pOut->w = pQ0->w * pQ1->w - pQ0->x * pQ1->x - pQ0->y * pQ1->y - pQ0->z * pQ1->z;
    return pOut;
}
struct quat* quatSetAxisAngle(struct quat* __restrict pOut, struct vec3* __restrict axis, float radian)
{
    float sn = sinf(radian * 0.5f);
    pOut->x = sn * axis->x;
    pOut->y = sn * axis->y;
    pOut->z = sn * axis->z;
    pOut->w = cosf(radian * 0.5f);
    return pOut;
}
struct quat* quatRotateX(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian)
{
    float sn = sinf(radian * 0.5f);
    float cn = cosf(radian * 0.5f);
    pOut->x = pQ->x * sn + pQ->w * cn;
    pOut->y = pQ->y * sn + pQ->z * cn;
    pOut->z = pQ->z * sn - pQ->y * cn;
    pOut->w = pQ->w * sn - pQ->x * cn;
    return pOut;
}
struct quat* quatRotateY(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian)
{
    float sn = sinf(radian * 0.5f);
    float cn = cosf(radian * 0.5f);
    pOut->x = pQ->x * cn - pQ->z * sn;
    pOut->y = pQ->y * cn + pQ->w * sn;
    pOut->z = pQ->z * cn + pQ->x * sn;
    pOut->w = pQ->w * cn - pQ->y * sn;
    return pOut;
}
struct quat* quatRotateZ(struct quat* __restrict pOut, struct quat* __restrict pQ, float radian)
{
    float sn = sinf(radian * 0.5f);
    float cn = cosf(radian * 0.5f);
    pOut->x = pQ->x * cn + pQ->y * sn;
    pOut->y = pQ->y * cn - pQ->x * sn;
    pOut->z = pQ->z * cn + pQ->w * sn;
    pOut->w = pQ->w * cn - pQ->z * sn;
    return pOut;
}
struct quat* quatCalcW(struct quat* __restrict pOut, struct quat* __restrict pQ)
{
    pOut->x = pQ->x;
    pOut->y = pQ->y;
    pOut->z = pQ->z;
    pOut->w = sqrtf(fabsf(1.0f - pQ->x * pQ->x - pQ->y * pQ->y - pQ->z * pQ->z));
    return pOut;
}
struct quat* quatInvert(struct quat* __restrict pOut, struct quat* __restrict pQ)
{
    float dot = pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w;
    if (dot) dot = 1.0f / dot;
    pOut->x = -pQ->x * dot;
    pOut->y = -pQ->y * dot;
    pOut->z = -pQ->z * dot;
    pOut->w = pQ->w * dot;
    return pOut;
}
struct quat* quatConjugate(struct quat* __restrict pOut, struct quat* __restrict pQ)
{
    pOut->x = -pQ->x;
    pOut->y = -pQ->y;
    pOut->z = -pQ->z;
    pOut->w = pQ->w;
    return pOut;
}
struct mat4* quatToMat4(struct mat4* __restrict pOut, struct quat* __restrict pQ)
{
    float x2 = pQ->x + pQ->x;
    float y2 = pQ->y + pQ->y;
    float z2 = pQ->z + pQ->z;
    float xx = pQ->x * x2;
    float yx = pQ->y * x2;
    float yy = pQ->y * y2;
    float zx = pQ->z * x2;
    float zy = pQ->z * y2;
    float zz = pQ->z * z2;
    float wx = pQ->w * x2;
    float wy = pQ->w * y2;
    float wz = pQ->w * z2;
    
    pOut->data[0] = 1.0f - yy - zz;
    pOut->data[1] = yx + wz;
    pOut->data[2] = zx - wy;
    pOut->data[3] = 0.0f;
    pOut->data[4] = yx - wz;
    pOut->data[5] = 1.0f - xx - zz;
    pOut->data[6] = zy + wx;
    pOut->data[7] = 0.0f;
    pOut->data[8] = zx + wy;
    pOut->data[9] = zy - wx;
    pOut->data[10] = 1.0f - xx - yy;
    pOut->data[11] = 0.0f;
    pOut->data[12] = 0.0f;
    pOut->data[13] = 0.0f;
    pOut->data[14] = 0.0f;
    pOut->data[15] = 1.0f;
    return pOut;
}
float quatGetAxisAngle(struct vec3* __restrict pOutAxis, struct quat* __restrict pQ)
{
    float radian = acosf(pQ->w) * 2.0f;
    float sn = sinf(radian * 0.5f);
    if (sn != 0.0f)
    {
        pOutAxis->x = pQ->x / sn;
        pOutAxis->y = pQ->y / sn;
        pOutAxis->z = pQ->z / sn;
    }
    else
    {
        pOutAxis->x = 1.0f;
        pOutAxis->y = 0.0f;
        pOutAxis->z = 0.0f;
    }
    return radian;
}
#if defined(__APPLE__)
#pragma clang diagnostic pop
#endif

#endif // !_MATH_H_

