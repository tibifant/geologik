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

lsResult quaternion::SquadSetup(_Out_ vec *pA, _Out_ vec *pB, _Out_ vec *pC, const quaternion &q0, const quaternion &q1, const quaternion &q2, const quaternion &q3) const
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pA == nullptr || pB == nullptr || pC == nullptr, lsR_ArgumentNull);
  DirectX::XMQuaternionSquadSetup(&pA->v, &pB->v, &pC->v, q0.q, q1.q, q2.q, q3.q);

  goto epilogue;
epilogue:
  return result;
}

lsResult quaternion::ToAxisAngle(_Out_ vec *pAxis, _Out_ float_t *pAngle) const
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pAxis == nullptr || pAngle == nullptr, lsR_ArgumentNull);
  DirectX::XMQuaternionToAxisAngle(&pAxis->v, pAngle, q);

  goto epilogue;
epilogue:
  return result;
}

lsResult quaternion::GetAverageEst(_In_ const quaternion *pValues, const size_t count, _Out_ quaternion *pAverage)
{
  if (pValues == nullptr || pAverage == nullptr)
    return lsR_ArgumentNull;

  vec avg(0, 0, 0, 0);

  for (size_t i = 0; i < count; i++)
  {
    const quaternion &q = pValues[i];

    if (i > 0 && q.Dot(pValues[0]) < 0.f)
      avg += -vec(q.q);
    else
      avg += vec(q.q);
  }

  *pAverage = quaternion(avg.v).Normalize();

  return lsR_Success;
}

vec3f quaternion::ToEulerAngles() const
{
  // x-axis rotation
  const float_t sinr_cosp = 2.0f * (w * x + y * z);
  const float_t cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
  const float_t roll = atan2(sinr_cosp, cosr_cosp);

  // y-axis rotation
  const float_t sinp = 2.0f * (w * y - z * x);
  float_t pitch;

  if (lsAbs(sinp) >= 1)
    pitch = copysignf(lsHALFPIf, sinp);
  else
    pitch = asinf(sinp);

  // z-axis rotation
  const float_t siny_cosp = 2.0f * (w * z + x * y);
  const float_t cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
  const float_t yaw = atan2(siny_cosp, cosy_cosp);

  return vec3f(yaw, pitch, roll);
}

//////////////////////////////////////////////////////////////////////////

lsResult matrix::Decompose(_Out_ vec *pOutScale, _Out_ quaternion *pOutRotQuat, _Out_ vec *pOutTrans) const
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pOutScale == nullptr || pOutRotQuat == nullptr || pOutTrans == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(DirectX::XMMatrixDecompose(&pOutScale->v, &pOutRotQuat->q, &pOutTrans->v, m), lsR_InternalError);

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
