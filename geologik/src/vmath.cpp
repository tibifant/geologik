#include "vmath.h"

lsResult __vectorcall vec::TransformCoordStream2(_Out_ DirectX::XMFLOAT2 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT2 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutputData == nullptr || pInputData == nullptr, lsR_ArgumentNull);
  DirectX::XMVector2TransformCoordStream(pOutputData, outputStride, pInputData, inputStride, inputLength, matrix.m);

  goto epilogue;
epilogue:
  return result;
}

lsResult __vectorcall vec::TransformNormalStream2(_Out_ DirectX::XMFLOAT2 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT2 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutputData == nullptr || pInputData == nullptr, lsR_ArgumentNull);
  DirectX::XMVector2TransformNormalStream(pOutputData, outputStride, pInputData, inputStride, inputLength, matrix.m);

  goto epilogue;
epilogue:
  return result;
}

lsResult __vectorcall vec::TransformCoordStream3(_Out_ DirectX::XMFLOAT3 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT3 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutputData == nullptr || pInputData == nullptr, lsR_ArgumentNull);
  DirectX::XMVector3TransformCoordStream(pOutputData, outputStride, pInputData, inputStride, inputLength, matrix.m);

  goto epilogue;
epilogue:
  return result;
}

lsResult __vectorcall vec::TransformNormalStream3(_Out_ DirectX::XMFLOAT3 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT3 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutputData == nullptr || pInputData == nullptr, lsR_ArgumentNull);
  DirectX::XMVector3TransformNormalStream(pOutputData, outputStride, pInputData, inputStride, inputLength, matrix.m);

  goto epilogue;
epilogue:
  return result;
}

lsResult __vectorcall vec::ComponentsFromNormal3(_Out_ vec *pParallel, _Out_ vec *pPerpendicular, const vec &v, const vec &normal)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pParallel == nullptr || pParallel == nullptr, lsR_ArgumentNull);
  DirectX::XMVector3ComponentsFromNormal(&pParallel->v, &pPerpendicular->v, v.v, normal.v);

  goto epilogue;
epilogue:
  return result;
}

lsResult __vectorcall vec::TransformStream4(_Out_ DirectX::XMFLOAT4 *pOutputData, const size_t outputStride, _In_ DirectX::XMFLOAT4 *pInputData, const size_t inputStride, const size_t inputLength, const matrix &matrix)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutputData == nullptr || pInputData == nullptr, lsR_ArgumentNull);
  DirectX::XMVector4TransformStream(pOutputData, outputStride, pInputData, inputStride, inputLength, matrix.m);

  goto epilogue;
epilogue:
  return result;
}

matrix vec::OuterProduct4(const vec a, const vec b)
{
  matrix m;

  for (size_t x = 0; x < 4; x++)
    for (size_t y = 0; y < 4; y++)
      m._m[x][y] = a._v[x] * b._v[y];

  return m;
}

//////////////////////////////////////////////////////////////////////////

void quaternion::SquadSetup(_Out_ vec *pA, _Out_ vec *pB, _Out_ vec *pC, const quaternion &q0, const quaternion &q1, const quaternion &q2, const quaternion &q3) const
{
  lsAssert(!(pA == nullptr || pB == nullptr || pC == nullptr));
  DirectX::XMQuaternionSquadSetup(&pA->v, &pB->v, &pC->v, q0.q, q1.q, q2.q, q3.q);
}

void quaternion::ToAxisAngle(_Out_ vec *pAxis, _Out_ float_t *pAngle) const
{
  lsAssert(!(pAxis == nullptr || pAngle == nullptr));
  DirectX::XMQuaternionToAxisAngle(&pAxis->v, pAngle, q);
}

quaternion quaternion::GetAverageEst(_In_ const quaternion *pValues, const size_t count)
{
  lsAssert(pValues != nullptr);

  vec avg(0, 0, 0, 0);

  for (size_t i = 0; i < count; i++)
  {
    const quaternion &q = pValues[i];

    if (i > 0 && q.Dot(pValues[0]) < 0.f)
      avg += -vec(q.q);
    else
      avg += vec(q.q);
  }

  return quaternion(avg.v).Normalize();
}

vec3f quaternion::ToEulerAngles() const // Tait–Bryan
{
  // See: https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/ (somehow works flawlessly - so far)
  const vec sqr(DirectX::XMVectorMultiply(q, q));
  const float_t unit = sqr.x + sqr.y + sqr.z + sqr.w; // if normalised is one, otherwise is correction factor
  const float_t test = x * y + z * w;

  if (test > 0.499f * unit) // singularity at north pole (brrr!)
    return vec3f(2.f * lsATan2(x, w), lsHALFPIf, 0);
  else if (test < -0.499f * unit) // singularity at south pole (brrr!)
    return vec3f(-2.f * lsATan2(x, w), -lsHALFPIf, 0);
  else
    return vec3f(lsATan2(2.f * y * w - 2.f * x * z, sqr.x - sqr.y - sqr.z + sqr.w), lsASin(2.f * test / unit), lsATan2(2.f * x * w - 2.f * y * z, -sqr.x + sqr.y - sqr.z + sqr.w));
}

// https://stackoverflow.com/a/27496984
inline vec3f euler_from_three_axis_internal(const float_t r11, const float_t r12, const float_t r21, const float_t r31, const float_t r32)
{
  return vec3f(lsATan2(r31, r32), lsASin(r21), lsATan2(r11, r12));
}

vec3f quaternion::ToEulerAnglesXYZ() const
{
  return euler_from_three_axis_internal(
    -2.f * (x * y - w * z),
    w * w - x * x + y * y - z * z,
    2 * (y * z + w * x),
    -2.f * (x * z - w * y),
    w * w - x * x - y * y + z * z
  );
}

vec3f quaternion::ToEulerAnglesZYX() const
{
  return euler_from_three_axis_internal(
    2.f * (x * y + w * z),
    w * w + x * x - y * y - z * z,
    -2.f * (x * z - w * y),
    2.f * (y * z + w * x),
    w * w - x * x - y * y + z * z
  );
}

//////////////////////////////////////////////////////////////////////////

lsResult matrix::Decompose(_Out_ vec *pOutScale, _Out_ quaternion *pOutRotQuat, _Out_ vec *pOutTrans) const
{
  lsResult result = lsR_Success;

  lsAssert(pOutScale != nullptr && pOutRotQuat != nullptr && pOutTrans != nullptr);
  LS_ERROR_IF(!DirectX::XMMatrixDecompose(&pOutScale->v, &pOutRotQuat->q, &pOutTrans->v, m), lsR_InternalError);

  goto epilogue;
epilogue:
  return result;
}

matrix matrix::AddComponentWise(const matrix &a, const matrix &b)
{
  matrix ret;

  for (size_t i = 0; i < 4; i++)
    for (size_t j = 0; j < 4; j++)
      ret._m[i][j] = a._m[i][j] + b._m[i][j];

  return ret;
}

matrix matrix::SubtractComponentWise(const matrix &a, const matrix &b)
{
  matrix ret;

  for (size_t i = 0; i < 4; i++)
    for (size_t j = 0; j < 4; j++)
      ret._m[i][j] = a._m[i][j] - b._m[i][j];

  return ret;
}

matrix matrix::MultiplyComponentWise(const matrix &a, const matrix &b)
{
  matrix ret;

  for (size_t i = 0; i < 4; i++)
    for (size_t j = 0; j < 4; j++)
      ret._m[i][j] = a._m[i][j] * b._m[i][j];

  return ret;
}

matrix matrix::DivideComponentWise(const matrix &a, const matrix &b)
{
  matrix ret;

  for (size_t i = 0; i < 4; i++)
    for (size_t j = 0; j < 4; j++)
      ret._m[i][j] = a._m[i][j] / b._m[i][j];

  return ret;
}
