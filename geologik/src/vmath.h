#pragma once

#include "core.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>

//////////////////////////////////////////////////////////////////////////

struct matrix;
struct quaternion;

struct vec
{
#pragma warning(push) 
#pragma warning(disable: 4201)
  union
  {
    DirectX::XMVECTOR v;
    struct { float_t x, y, z, w; };
    float_t _v[4];
  };
#pragma warning(pop)

  inline vec() :
    x(0),
    y(0),
    z(0),
    w(1)
  { }

  inline vec(const vec &a) :
    v(a.v)
  { }

  inline explicit vec(const float_t s)
  {
    const DirectX::XMFLOAT4 f(s, s, s, s);
    v = DirectX::XMLoadFloat4(&f);
  }

  inline static vec Scalar(const float_t s) { return vec(DirectX::XMLoadFloat(&s)); }

  inline vec(const float_t x, const float_t y)
  {
    const DirectX::XMFLOAT2 f(x, y);
    v = DirectX::XMLoadFloat2(&f);
  }

  inline vec(const float_t x, const float_t y, const float_t z)
  {
    const DirectX::XMFLOAT3 f(x, y, z);
    v = DirectX::XMLoadFloat3(&f);
  }

  inline vec(const float_t x, const float_t y, const float_t z, const float_t w)
  {
    const DirectX::XMFLOAT4 f(x, y, z, w);
    v = DirectX::XMLoadFloat4(&f);
  }

  inline vec(const vec2f &v) : vec(v.x, v.y) { }
  inline vec(const vec3f &v) : vec(v.x, v.y, v.z) { }
  inline vec(const vec4f &v) : vec(v.x, v.y, v.z, v.w) { }

  inline vec(DirectX::XMVECTOR _v) : v(_v) { }

  inline vec(DirectX::XMFLOAT2 _v) { v = DirectX::XMLoadFloat2(&_v); }
  inline vec(DirectX::XMFLOAT3 _v) { v = DirectX::XMLoadFloat3(&_v); }
  inline vec(DirectX::XMFLOAT4 _v) { v = DirectX::XMLoadFloat4(&_v); }

  inline vec operator+(const vec &a) const { return vec(DirectX::XMVectorAdd(v, a.v)); }
  inline vec operator-(const vec &a) const { return vec(DirectX::XMVectorSubtract(v, a.v)); }
  inline vec operator*(const vec &a) const { return vec(DirectX::XMVectorMultiply(v, a.v)); }
  inline vec operator/(const vec &a) const { return vec(DirectX::XMVectorDivide(v, a.v)); }

  inline vec &operator+=(const vec &a) { return *this = (*this + a); }
  inline vec &operator-=(const vec &a) { return *this = (*this - a); }
  inline vec &operator*=(const vec &a) { return *this = (*this * a); }
  inline vec &operator/=(const vec &a) { return *this = (*this / a); }

  inline vec operator*(const float_t a) const { return vec(DirectX::XMVectorScale(v, a)); }
  inline vec operator/(const float_t a) const { return vec(DirectX::XMVectorScale(v, 1.0f / a)); }

  inline vec &operator*=(const float_t a) { return *this = (*this * a); }
  inline vec &operator/=(const float_t a) { return *this = (*this / a); }

  inline vec operator-() const { return vec(DirectX::XMVectorNegate(v)); }

  inline operator vec2f() const
  {
    DirectX::XMFLOAT2 f;
    DirectX::XMStoreFloat2(&f, v);
    return vec2f(f.x, f.y);
  }

  inline operator DirectX::XMFLOAT2() const
  {
    DirectX::XMFLOAT2 f;
    DirectX::XMStoreFloat2(&f, v);
    return f;
  }

  inline operator DirectX::XMFLOAT3() const
  {
    DirectX::XMFLOAT3 f;
    DirectX::XMStoreFloat3(&f, v);
    return f;
  }

  inline operator DirectX::XMFLOAT4() const
  {
    DirectX::XMFLOAT4 f;
    DirectX::XMStoreFloat4(&f, v);
    return f;
  }

  inline vec4t<bool> ComponentEquals(const vec &a) const
  {
    DirectX::XMVECTOR __v = DirectX::XMVectorEqual(v, a.v);
    DirectX::XMUINT4 f;
    DirectX::XMStoreUInt4(&f, __v);
    return vec4t<bool>(f.x != 0, f.y != 0, f.z != 0, f.w != 0);
  }

  inline vec Abs() const { return vec(DirectX::XMVectorAbs(v)); }

  inline vec AngleBetweenNormals2(const vec &a) const { return vec(DirectX::XMVector2AngleBetweenNormals(v, a.v)); }
  inline vec AngleBetweenNormalsEst2(const vec &a) const { return vec(DirectX::XMVector2AngleBetweenNormalsEst(v, a.v)); }
  inline vec AngleBetweenVectors2(const vec &a) const { return vec(DirectX::XMVector2AngleBetweenVectors(v, a.v)); }
  inline vec ClampLength2(const float_t min, const float_t max) const { return vec(DirectX::XMVector2ClampLength(v, min, max)); }
  inline vec ClampLengthVectors2(const vec &min, const vec &max) const { return vec(DirectX::XMVector2ClampLengthV(v, min.v, max.v)); }
  inline float_t Dot2(const vec &a) const { return vec(DirectX::XMVector2Dot(v, a.v)).x; }
  inline vec Cross2(const vec &a) const { return vec(DirectX::XMVector2Cross(v, a.v)); }
  inline bool Equals2(const vec &a) const { return DirectX::XMVector2Equal(v, a.v); }
  inline bool NotEqualTo2(const vec &a) const { return DirectX::XMVector2NotEqual(v, a.v); }
  inline bool EqualsApproximately2(const vec &a, const vec &epsilon) const { return DirectX::XMVector2NearEqual(v, a.v, epsilon.v); }
  inline bool Greater2(const vec &a) const { return DirectX::XMVector2Greater(v, a.v); }
  inline bool GreaterOrEqual2(const vec &a) const { return DirectX::XMVector2GreaterOrEqual(v, a.v); }
  inline bool Less2(const vec &a) const { return DirectX::XMVector2Less(v, a.v); }
  inline bool LessOrEqual2(const vec &a) const { return DirectX::XMVector2LessOrEqual(v, a.v); }
  inline bool InBounds2(const vec &a) const { return DirectX::XMVector2InBounds(v, a.v); }

  inline static vec __vectorcall Min(const vec a, const vec b) { return vec(DirectX::XMVectorMin(a.v, b.v)); }
  inline static vec __vectorcall Max(const vec a, const vec b) { return vec(DirectX::XMVectorMax(a.v, b.v)); }

  inline static vec __vectorcall Lerp(const vec a, const vec b, const float_t t) { return vec(DirectX::XMVectorLerp(a.v, b.v, t)); }
  inline static vec __vectorcall LerpVector(const vec a, const vec b, const vec t) { return vec(DirectX::XMVectorLerpV(a.v, b.v, t.v)); }
  inline static vec __vectorcall Barycentric(const vec a, const vec b, const vec c, const float_t f, const float_t g) { return vec(DirectX::XMVectorBaryCentric(a.v, b.v, c.v, f, g)); }
  inline static vec __vectorcall BarycentricVector(const vec a, const vec b, const vec c, const vec f, const vec g) { return vec(DirectX::XMVectorBaryCentricV(a.v, b.v, c.v, f.v, g.v)); }
  inline static vec __vectorcall CatmullRom(const vec a, const vec b, const vec c, const vec d, const float_t f) { return vec(DirectX::XMVectorCatmullRom(a.v, b.v, c.v, d.v, f)); }
  inline static vec __vectorcall CatmullRomVector(const vec a, const vec b, const vec c, const vec d, const vec f) { return vec(DirectX::XMVectorCatmullRomV(a.v, b.v, c.v, d.v, f.v)); }
  inline static vec __vectorcall Hermite(const vec v1, const vec t1, const vec v2, const vec t2, const float_t f) { return vec(DirectX::XMVectorHermite(v1.v, t1.v, v2.v, t2.v, f)); }
  inline static vec __vectorcall HermiteVector(const vec v1, const vec t1, const vec v2, const vec t2, const  vec &f) { return vec(DirectX::XMVectorHermiteV(v1.v, t1.v, v2.v, t2.v, f.v)); }

  inline static vec __vectorcall IntersectLine2(const vec line1Point1, const vec line1Point2, const vec line2Point1, const vec line2Point2) { return vec(DirectX::XMVector2IntersectLine(line1Point1.v, line1Point2.v, line2Point1.v, line2Point2.v)); }
  inline static float_t __vectorcall LinePointDistance2(const vec line1Point1, const vec line1Point2, const vec point) { return vec(DirectX::XMVector2LinePointDistance(line1Point1.v, line1Point2.v, point.v)).x; }

  inline float_t Length2() const { return vec(DirectX::XMVector2Length(v)).x; }
  inline float_t LengthEst2() const { return vec(DirectX::XMVector2LengthEst(v)).x; }
  inline float_t LengthSquared2() const { return vec(DirectX::XMVector2LengthSq(v)).x; }
  inline vec Normalize2() const { return vec(DirectX::XMVector2Normalize(v)); }
  inline vec NormalizeEst2() const { return vec(DirectX::XMVector2NormalizeEst(v)); }
  inline vec Orthogonal2() const { return vec(DirectX::XMVector2Orthogonal(v)); }
  inline vec ReciprocalLength2() const { return vec(DirectX::XMVector2ReciprocalLength(v)); }
  inline vec ReciprocalLengthEst2() const { return vec(DirectX::XMVector2ReciprocalLengthEst(v)); }

  inline static vec __vectorcall Reflect2(const vec incident, const vec normal) { return vec(DirectX::XMVector2Reflect(incident.v, normal.v)); }
  inline static vec __vectorcall Refract2(const vec incident, const vec normal, const float_t refractionIndex) { return vec(DirectX::XMVector2Refract(incident.v, normal.v, refractionIndex)); }
  inline static vec __vectorcall RefractVector2(const vec incident, const vec normal, const vec refractionIndex) { return vec(DirectX::XMVector2RefractV(incident.v, normal.v, refractionIndex.v)); }

  vec Transform2(const matrix &matrix) const;
  vec TransformCoord2(const matrix &matrix) const;
  vec TransformNormal2(const matrix &matrix) const;

  inline vec AngleBetweenNormals3(const vec &a) const { return vec(DirectX::XMVector3AngleBetweenNormals(v, a.v)); }
  inline vec AngleBetweenNormalsEst3(const vec &a) const { return vec(DirectX::XMVector3AngleBetweenNormalsEst(v, a.v)); }
  inline vec AngleBetweenVectors3(const vec &a) const { return vec(DirectX::XMVector3AngleBetweenVectors(v, a.v)); }
  inline vec ClampLength3(const float_t min, const float_t max) const { return vec(DirectX::XMVector3ClampLength(v, min, max)); }
  inline vec ClampLengthVectors3(const vec &min, const vec &max) const { return vec(DirectX::XMVector3ClampLengthV(v, min.v, max.v)); }
  inline float_t Dot3(const vec &a) const { return vec(DirectX::XMVector3Dot(v, a.v)).x; }
  inline vec Cross3(const vec &a) const { return vec(DirectX::XMVector3Cross(v, a.v)); }

  inline bool Equals3(const vec &a) const { return DirectX::XMVector3Equal(v, a.v); }
  inline bool NotEqualTo3(const vec &a) const { return DirectX::XMVector3NotEqual(v, a.v); }
  inline bool EqualsApproximately3(const vec &a, const vec &epsilon) const { return DirectX::XMVector3NearEqual(v, a.v, epsilon.v); }
  inline bool Greater3(const vec &a) const { return DirectX::XMVector3Greater(v, a.v); }
  inline bool GreaterOrEqual3(const vec &a) const { return DirectX::XMVector3GreaterOrEqual(v, a.v); }
  inline bool Less3(const vec &a) const { return DirectX::XMVector3Less(v, a.v); }
  inline bool LessOrEqual3(const vec &a) const { return DirectX::XMVector3LessOrEqual(v, a.v); }
  inline bool InBounds3(const vec &a) const { return DirectX::XMVector3InBounds(v, a.v); }

  inline static float_t LinePointDistance3(const vec &line1Point1, const vec &line1Point2, const vec &point) { return vec(DirectX::XMVector3LinePointDistance(line1Point1.v, line1Point2.v, point.v)).x; }

  inline float_t Length3() const { return vec(DirectX::XMVector3Length(v)).x; }
  inline float_t LengthEst3() const { return vec(DirectX::XMVector3LengthEst(v)).x; }
  inline float_t LengthSquared3() const { return vec(DirectX::XMVector3LengthSq(v)).x; }
  inline vec Normalize3() const { return vec(DirectX::XMVector3Normalize(v)); }
  inline vec NormalizeEst3() const { return vec(DirectX::XMVector3NormalizeEst(v)); }
  inline vec Orthogonal3() const { return vec(DirectX::XMVector3Orthogonal(v)); }
  inline vec ReciprocalLength3() const { return vec(DirectX::XMVector3ReciprocalLength(v)); }
  inline vec ReciprocalLengthEst3() const { return vec(DirectX::XMVector3ReciprocalLengthEst(v)); }

  inline static vec __vectorcall Reflect3(const vec incident, const vec normal) { return vec(DirectX::XMVector3Reflect(incident.v, normal.v)); }
  inline static vec __vectorcall Refract3(const vec incident, const vec normal, const float_t refractionIndex) { return vec(DirectX::XMVector3Refract(incident.v, normal.v, refractionIndex)); }
  inline static vec __vectorcall RefractVector3(const vec incident, const vec normal, const vec refractionIndex) { return vec(DirectX::XMVector3RefractV(incident.v, normal.v, refractionIndex.v)); }

  vec Transform3(const matrix &matrix) const;
  vec TransformCoord3(const matrix &matrix) const;
  vec TransformNormal3(const matrix &matrix) const;
  vec Rotate3(const quaternion &quaternion) const;
  vec RotateInverse3(const quaternion &quaternion) const;

  inline vec AngleBetweenNormals4(const vec &a) const { return vec(DirectX::XMVector4AngleBetweenNormals(v, a.v)); }
  inline vec AngleBetweenNormalsEst4(const vec &a) const { return vec(DirectX::XMVector4AngleBetweenNormalsEst(v, a.v)); }
  inline vec AngleBetweenVectors4(const vec &a) const { return vec(DirectX::XMVector4AngleBetweenVectors(v, a.v)); }
  inline vec ClampLength4(const float_t min, const float_t max) const { return vec(DirectX::XMVector4ClampLength(v, min, max)); }
  inline vec ClampLengthVectors4(const vec &min, const vec &max) const { return vec(DirectX::XMVector4ClampLengthV(v, min.v, max.v)); }
  inline float_t Dot4(const vec &a) const { return vec(DirectX::XMVector4Dot(v, a.v)).x; }
  inline vec Cross4(const vec &a, const vec &b) const { return vec(DirectX::XMVector4Cross(v, a.v, b.v)); }
  inline bool Equals4(const vec &a) const { return DirectX::XMVector4Equal(v, a.v); }
  inline bool NotEqualTo4(const vec &a) const { return DirectX::XMVector4NotEqual(v, a.v); }
  inline bool EqualsApproximately4(const vec &a, const vec &epsilon) const { return DirectX::XMVector4NearEqual(v, a.v, epsilon.v); }
  inline bool Greater4(const vec &a) const { return DirectX::XMVector4Greater(v, a.v); }
  inline bool GreaterOrEqual4(const vec &a) const { return DirectX::XMVector4GreaterOrEqual(v, a.v); }
  inline bool Less4(const vec &a) const { return DirectX::XMVector4Less(v, a.v); }
  inline bool LessOrEqual4(const vec &a) const { return DirectX::XMVector4LessOrEqual(v, a.v); }
  inline bool InBounds4(const vec &a) const { return DirectX::XMVector4InBounds(v, a.v); }
  inline float_t Length4() const { return vec(DirectX::XMVector4Length(v)).x; }
  inline float_t LengthEst4() const { return vec(DirectX::XMVector4LengthEst(v)).x; }
  inline float_t LengthSquared4() const { return vec(DirectX::XMVector4LengthSq(v)).x; }
  inline vec Normalize4() const { return vec(DirectX::XMVector4Normalize(v)); }
  inline vec NormalizeEst4() const { return vec(DirectX::XMVector4NormalizeEst(v)); }
  inline vec Orthogonal4() const { return vec(DirectX::XMVector4Orthogonal(v)); }
  inline vec ReciprocalLength4() const { return vec(DirectX::XMVector4ReciprocalLength(v)); }
  inline vec ReciprocalLengthEst4() const { return vec(DirectX::XMVector4ReciprocalLengthEst(v)); }

  inline static vec __vectorcall Reflect4(const vec &incident, const vec &normal) { return vec(DirectX::XMVector4Reflect(incident.v, normal.v)); }
  inline static vec __vectorcall Refract4(const vec &incident, const vec &normal, const float_t refractionIndex) { return vec(DirectX::XMVector4Refract(incident.v, normal.v, refractionIndex)); }
  inline static vec __vectorcall RefractVector4(const vec &incident, const vec &normal, const vec &refractionIndex) { return vec(DirectX::XMVector4RefractV(incident.v, normal.v, refractionIndex.v)); }

  vec Transform4(const matrix &matrix) const;

  inline static float_t Dot2(const vec &a, const vec &b) { return a.Dot2(b); }
  inline static vec Cross2(const vec &a, const vec &b) { return a.Cross2(b); }

  inline static float_t Dot3(const vec &a, const vec &b) { return a.Dot3(b); }
  inline static vec Cross3(const vec &a, const vec &b) { return a.Cross3(b); }

  inline static float_t Dot4(const vec &a, const vec &b) { return a.Dot4(b); }
  inline static vec Cross4(const vec &a, const vec &b, const vec &c) { return a.Cross4(b, c); }

  static lsResult __vectorcall TransformCoordStream2(_Out_ DirectX::XMFLOAT2 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT2 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix);
  static lsResult __vectorcall TransformNormalStream2(_Out_ DirectX::XMFLOAT2 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT2 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix);

  static lsResult __vectorcall TransformCoordStream3(_Out_ DirectX::XMFLOAT3 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT3 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix);
  static lsResult __vectorcall TransformNormalStream3(_Out_ DirectX::XMFLOAT3 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT3 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix);
  static lsResult __vectorcall ComponentsFromNormal3(_Out_ vec *pParallel, _Out_ vec *pPerpendicular, const vec &v, const vec &normal);

  static lsResult __vectorcall TransformStream4(_Out_ DirectX::XMFLOAT4 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT4 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix);

  static matrix OuterProduct4(const vec a, const vec b);
};

//////////////////////////////////////////////////////////////////////////

struct quaternion
{
#pragma warning(push) 
#pragma warning(disable: 4201)
  union
  {
    DirectX::XMVECTOR q;
    struct { float_t x, y, z, w; };
    float_t _q[4];
  };
#pragma warning(pop)

  inline quaternion() { *this = Identity(); }
  inline explicit quaternion(DirectX::XMVECTOR v) : q(v) { }

  inline quaternion Multiply(const quaternion &q2) const { return quaternion(DirectX::XMQuaternionMultiply(q, q2.q)); }

  inline quaternion operator*(const quaternion &q1) const { return Multiply(q1); };
  inline quaternion &operator*=(const quaternion &q1) { return *this = Multiply(q1); };

  inline static quaternion BaryCentric(const quaternion &q0, const quaternion &q1, const quaternion &q2, const float_t f, const float_t g) { return quaternion(DirectX::XMQuaternionBaryCentric(q0.q, q1.q, q2.q, f, g)); }
  inline static quaternion BaryCentricV(const quaternion &q0, const quaternion &q1, const quaternion &q2, const vec &f, const vec &g) { return quaternion(DirectX::XMQuaternionBaryCentricV(q0.q, q1.q, q2.q, f.v, g.v)); }

  inline quaternion Conjugate() const { return quaternion(DirectX::XMQuaternionConjugate(q)); }
  inline float_t Dot(const quaternion &q2) const { return quaternion(DirectX::XMQuaternionDot(q, q2.q)).x; }
  inline bool Equals(const quaternion &q2) const { return DirectX::XMQuaternionEqual(q, q2.q); }
  inline quaternion Exp() const { return quaternion(DirectX::XMQuaternionExp(q)); }
  inline quaternion Inverse() const { return quaternion(DirectX::XMQuaternionInverse(q)); }
  inline bool IsIdentity() const { return DirectX::XMQuaternionIsIdentity(q); }
  inline float_t Length() const { return quaternion(DirectX::XMQuaternionLength(q)).x; }
  inline float_t LengthSq() const { return quaternion(DirectX::XMQuaternionLengthSq(q)).x; }
  inline quaternion Ln() const { return quaternion(DirectX::XMQuaternionLn(q)); }
  inline quaternion Normalize() const { return quaternion(DirectX::XMQuaternionNormalize(q)); }
  inline quaternion NormalizeEst() const { return quaternion(DirectX::XMQuaternionNormalizeEst(q)); }
  inline bool NotEqualTo(const quaternion &q2) const { return DirectX::XMQuaternionNotEqual(q, q2.q); }
  inline vec ReciprocalLength() const { return vec(DirectX::XMQuaternionReciprocalLength(q)); }

  inline static quaternion Identity() { return quaternion(DirectX::XMQuaternionIdentity()); }

  static quaternion __vectorcall quaternion::FromRotationMatrix(const matrix &m);

  inline static quaternion __vectorcall FromRotationAxis(const vec axis, const float_t angle) { return quaternion(DirectX::XMQuaternionRotationAxis(axis.v, angle)); }
  inline static quaternion __vectorcall FromRotationNormal(const vec normalAxis, const float_t angle) { return quaternion(DirectX::XMQuaternionRotationNormal(normalAxis.v, angle)); }
  inline static quaternion __vectorcall FromRotationRollPitchYaw(const float_t pitch, const float_t yaw, const float_t roll) { return quaternion(DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll)); }
  inline static quaternion __vectorcall FromRotationRollPitchYaw(const vec angles) { return quaternion(DirectX::XMQuaternionRotationRollPitchYawFromVector(angles.v)); }

  inline static quaternion __vectorcall Slerp(const quaternion q0, const quaternion q1, const float_t t) { return quaternion(DirectX::XMQuaternionSlerp(q0.q, q1.q, t)); }
  inline static quaternion __vectorcall SlerpV(const quaternion q0, const quaternion q1, const vec t) { return quaternion(DirectX::XMQuaternionSlerpV(q0.q, q1.q, t.v)); }
  inline static quaternion __vectorcall Squad(const quaternion q0, const quaternion q1, const quaternion q2, const quaternion q3, const float_t t) { return quaternion(DirectX::XMQuaternionSquad(q0.q, q1.q, q2.q, q3.q, t)); }
  inline static quaternion __vectorcall SquadV(const quaternion q0, const quaternion q1, const quaternion q2, const quaternion q3, const vec t) { return quaternion(DirectX::XMQuaternionSquadV(q0.q, q1.q, q2.q, q3.q, t.v)); }

  vec3f ToEulerAngles() const;
  lsResult SquadSetup(_Out_ vec *pA, _Out_ vec *pB, _Out_ vec *pC, const quaternion &q0, const quaternion &q1, const quaternion &q2, const quaternion &q3) const;
  lsResult ToAxisAngle(_Out_ vec *pAxis, _Out_ float_t *pAngle) const;

  // This gets very close to the exact value that is way more expensive to calculate.
  static lsResult GetAverageEst(_In_ const quaternion *pValues, const size_t count, _Out_ quaternion *pAverage);
};

//////////////////////////////////////////////////////////////////////////

struct matrix
{
#pragma warning(push) 
#pragma warning(disable: 4201)
  union
  {
    DirectX::XMMATRIX m;
    struct
    {
      float_t _11, _12, _13, _14;
      float_t _21, _22, _23, _24;
      float_t _31, _32, _33, _34;
      float_t _41, _42, _43, _44;
    };
    float_t _m[4][4];
  };
#pragma warning(pop)


  inline matrix() { m = DirectX::XMMatrixIdentity(); }
  inline matrix(DirectX::XMMATRIX _m) : m(_m) { }

  inline matrix operator*(const matrix &q1) const { return Multiply(q1); }
  inline matrix &operator*=(const matrix &q1) { return *this = Multiply(q1); }
  inline matrix &operator=(const matrix &copy) { m = copy.m; return *this; }

  inline static matrix __vectorcall Identity() { return matrix(DirectX::XMMatrixIdentity()); }
  inline static matrix __vectorcall AffineTransformation(const vec &scaling, const vec &rotationOrigin, const quaternion &rotationQuaternion, const vec &translation) { return matrix(DirectX::XMMatrixAffineTransformation(scaling.v, rotationOrigin.v, rotationQuaternion.q, translation.v)); }
  inline static matrix __vectorcall AffineTransformation2D(const vec &scaling, const vec &rotationOrigin, const float_t rotation, const vec &translation) { return matrix(DirectX::XMMatrixAffineTransformation2D(scaling.v, rotationOrigin.v, rotation, translation.v)); }

  lsResult Decompose(_Out_ vec *pOutScale, _Out_ quaternion *pOutRotQuat, _Out_ vec *pOutTrans) const;

  inline vec Determinant() const { return vec(DirectX::XMMatrixDeterminant(m)); }
  inline matrix Inverse(_Out_ vec *pDeterminant = nullptr) const { return matrix(DirectX::XMMatrixInverse(pDeterminant == nullptr ? nullptr : &pDeterminant->v, m)); }

  inline static matrix __vectorcall LookAtLH(const vec &eyePosition, const vec &focusPosition, const vec &upDirection) { return matrix(DirectX::XMMatrixLookAtLH(eyePosition.v, focusPosition.v, upDirection.v)); }
  inline static matrix __vectorcall LookAtRH(const vec &eyePosition, const vec &focusPosition, const vec &upDirection) { return matrix(DirectX::XMMatrixLookAtRH(eyePosition.v, focusPosition.v, upDirection.v)); }
  inline static matrix __vectorcall LookToLH(const vec &eyePosition, const vec &eyeDirection, const vec &upDirection) { return matrix(DirectX::XMMatrixLookToLH(eyePosition.v, eyeDirection.v, upDirection.v)); }
  inline static matrix __vectorcall LookToRH(const vec &eyePosition, const vec &eyeDirection, const vec &upDirection) { return matrix(DirectX::XMMatrixLookToRH(eyePosition.v, eyeDirection.v, upDirection.v)); }

  inline matrix Multiply(const matrix &m2) const { return matrix(DirectX::XMMatrixMultiply(m, m2.m)); }
  inline matrix MultiplyTranspose(const matrix &m2) const { return matrix(DirectX::XMMatrixMultiplyTranspose(m, m2.m)); }

  inline static matrix __vectorcall OrthographicLH(const float_t viewWidth, const float_t viewHeight, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ)); }
  inline static matrix __vectorcall OrthographicRH(const float_t viewWidth, const float_t viewHeight, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixOrthographicRH(viewWidth, viewHeight, nearZ, farZ)); }
  inline static matrix __vectorcall OrthographicOffCenterLH(const float_t viewLeft, const float_t viewRight, const float_t viewBottom, const float_t viewTop, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixOrthographicOffCenterLH(viewLeft, viewRight, viewBottom, viewTop, nearZ, farZ)); }
  inline static matrix __vectorcall OrthographicOffCenterRH(const float_t viewLeft, const float_t viewRight, const float_t viewBottom, const float_t viewTop, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixOrthographicOffCenterRH(viewLeft, viewRight, viewBottom, viewTop, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveLH(const float_t viewWidth, const float_t viewHeight, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveLH(viewWidth, viewHeight, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveRH(const float_t viewWidth, const float_t viewHeight, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveRH(viewWidth, viewHeight, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveFovLH(const float_t fovAngleY, const float_t aspectRatio, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveFovRH(const float_t fovAngleY, const float_t aspectRatio, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveOffCenterLH(const float_t viewLeft, const float_t viewRight, const float_t viewBottom, const float_t viewTop, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveOffCenterLH(viewLeft, viewRight, viewBottom, viewTop, nearZ, farZ)); }
  inline static matrix __vectorcall PerspectiveOffCenterRH(const float_t viewLeft, const float_t viewRight, const float_t viewBottom, const float_t viewTop, const float_t nearZ, const float_t farZ) { return matrix(DirectX::XMMatrixPerspectiveOffCenterRH(viewLeft, viewRight, viewBottom, viewTop, nearZ, farZ)); }
  inline static matrix __vectorcall Reflect(const vec &reflectionPlane) { return matrix(DirectX::XMMatrixReflect(reflectionPlane.v)); }
  inline static matrix __vectorcall RotationAxis(const vec &axis, const float_t angle) { return matrix(DirectX::XMMatrixRotationAxis(axis.v, angle)); }
  inline static matrix __vectorcall RotationQuaternion(const quaternion &quaternion) { return matrix(DirectX::XMMatrixRotationQuaternion(quaternion.q)); }
  inline static matrix __vectorcall RotationNormal(const vec &normalAxis, const float_t angle) { return matrix(DirectX::XMMatrixRotationNormal(normalAxis.v, angle)); }
  inline static matrix __vectorcall RotationRollPitchYaw(const float_t pitch, const float_t yaw, const float_t roll) { return matrix(DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)); }
  inline static matrix __vectorcall RotationRollPitchYawFromVector(const vec &angles) { return matrix(DirectX::XMMatrixRotationRollPitchYawFromVector(angles.v)); }
  inline static matrix __vectorcall RotationX(const float_t angle) { return matrix(DirectX::XMMatrixRotationX(angle)); }
  inline static matrix __vectorcall RotationY(const float_t angle) { return matrix(DirectX::XMMatrixRotationY(angle)); }
  inline static matrix __vectorcall RotationZ(const float_t angle) { return matrix(DirectX::XMMatrixRotationZ(angle)); }
  inline static matrix __vectorcall Scale(const float_t scaleX, const float_t scaleY, const float_t scaleZ) { return matrix(DirectX::XMMatrixScaling(scaleX, scaleY, scaleZ)); }
  inline static matrix __vectorcall ScalingFromVector(const vec &scale) { return matrix(DirectX::XMMatrixScalingFromVector(scale.v)); }
  inline static matrix __vectorcall Shadow(const vec &shadowPlane, const vec &lightPosition) { return matrix(DirectX::XMMatrixShadow(shadowPlane.v, lightPosition.v)); }
  inline static matrix __vectorcall Transformation(const vec &scalingOrigin, const quaternion &scalingOrientationQuaternion, const vec &scaling, const vec &rotationOrigin, const quaternion &rotationQuaternion, const vec &translation) { return matrix(DirectX::XMMatrixTransformation(scalingOrigin.v, scalingOrientationQuaternion.q, scaling.v, rotationOrigin.v, rotationQuaternion.q, translation.v)); }
  inline static matrix __vectorcall Transformation2D(const vec &scalingOrigin, const float_t scalingOrientation, const vec &scaling, const vec &rotationOrigin, const float_t rotation, const vec &translation) { return matrix(DirectX::XMMatrixTransformation2D(scalingOrigin.v, scalingOrientation, scaling.v, rotationOrigin.v, rotation, translation.v)); }
  inline static matrix __vectorcall Translation(const float_t offsetX, const float_t offsetY, const float_t offsetZ) { return matrix(DirectX::XMMatrixTranslation(offsetX, offsetY, offsetZ)); }
  inline static matrix __vectorcall TranslationFromVector(const vec &offset) { return matrix(DirectX::XMMatrixTranslationFromVector(offset.v)); }

  inline static matrix __vectorcall Zero()
  {
    DirectX::XMVECTOR v[4];

    v[0] = _mm_setzero_ps();
    v[1] = _mm_setzero_ps();
    v[2] = _mm_setzero_ps();
    v[3] = _mm_setzero_ps();

    return matrix(*reinterpret_cast<DirectX::XMMATRIX *>(v));
  }

  inline matrix Transpose() const { return matrix(DirectX::XMMatrixTranspose(m)); }
  inline vec __vectorcall Transforvec4(const vec vector4) { return vec(XMVector4Transform(vector4.v, m)); }
  inline vec __vectorcall Transforvec3(const vec vector3) { return vec(XMVector3Transform(vector3.v, m)); }

  static matrix AddComponentWise(const matrix &a, const matrix &b);
  static matrix SubtractComponentWise(const matrix &a, const matrix &b);
  static matrix MultiplyComponentWise(const matrix &a, const matrix &b);
  static matrix DivideComponentWise(const matrix &a, const matrix &b);
};

//////////////////////////////////////////////////////////////////////////

inline vec vec::Transform2(const matrix &matrix) const { return vec(DirectX::XMVector2Transform(v, (DirectX::FXMMATRIX)matrix.m)); }
inline vec vec::TransformCoord2(const matrix &matrix) const { return vec(DirectX::XMVector2TransformCoord(v, matrix.m)); }
inline vec vec::TransformNormal2(const matrix &matrix) const { return vec(DirectX::XMVector2TransformNormal(v, matrix.m)); }

inline vec vec::Transform3(const matrix &matrix) const { return vec(DirectX::XMVector3Transform(v, (DirectX::FXMMATRIX)matrix.m)); }
inline vec vec::TransformCoord3(const matrix &matrix) const { return vec(DirectX::XMVector3TransformCoord(v, matrix.m)); }
inline vec vec::TransformNormal3(const matrix &matrix) const { return vec(DirectX::XMVector3TransformNormal(v, matrix.m)); }
inline vec vec::Rotate3(const quaternion &quaternion) const { return vec(DirectX::XMVector3Rotate(v, quaternion.q)); }
inline vec vec::RotateInverse3(const quaternion &quaternion) const { return vec(DirectX::XMVector3InverseRotate(v, quaternion.q)); }

inline vec vec::Transform4(const matrix &matrix) const { return vec(DirectX::XMVector4Transform(v, (DirectX::FXMMATRIX)matrix.m)); };

