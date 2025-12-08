#pragma once

#define _USE_MATH_DEFINES 1

#include <stdint.h>
#include <climits>
#include <math.h>
#include <functional>
#include <type_traits>
#include <inttypes.h>
#include <float.h>
#include <memory.h>
#include <malloc.h>

#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <intrin.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <x86intrin.h>

#define __debugbreak() __builtin_trap()
#endif

#ifndef _In_
#define _In_
#endif

#ifndef _Out_
#define _Out_
#endif

#ifndef _In_Out_
#define _In_Out_
#endif

enum lsResult
{
  lsR_Success,

  lsR_Failure,
  lsR_InvalidParameter,
  lsR_ArgumentNull,
  lsR_IOFailure,
  lsR_ArgumentOutOfBounds,
  lsR_ResourceInvalid,
  lsR_ResourceIncompatible,
  lsR_ResourceNotFound,
  lsR_ResourceStateInvalid,
  lsR_ResourceAlreadyExists,
  lsR_ResourceBusy,
  lsR_MemoryAllocationFailure,
  lsR_OperationNotSupported,
  lsR_NotSupported,
  lsR_EndOfStream,
  lsR_InternalError,
};

#define LS_SUCCESS(errorCode) (errorCode == lsR_Success)
#define LS_FAILED(errorCode) (!(LS_SUCCESS(errorCode)))

#define LS_STRINGIFY(x) #x

#ifdef _DEBUG
#define LS_DEBUG_ONLY_BREAK() __debugbreak()
#else
#define LS_DEBUG_ONLY_BREAK() do { } while (0)
#endif

#define LS_ERROR_SET(errorCode) \
  do \
  { result = errorCode; \
    puts("Error (" LS_STRINGIFY(errorCode) ") in File '" __FILE__ "' (Line " LS_STRINGIFY(__LINE__) ")"); \
    LS_DEBUG_ONLY_BREAK(); \
    goto epilogue; \
  } while (0) 

#define LS_ERROR_IF(booleanExpression, errorCode) \
  do \
  { if (booleanExpression) \
      LS_ERROR_SET(errorCode); \
  } while (0)

#define LS_ERROR_CHECK(functionCall) \
  do \
  { result = (functionCall); \
    if (LS_FAILED(result)) \
      LS_ERROR_SET(result); \
  } while (0)

#ifdef _DEBUG
#define lsAssert(a) \
  do \
  { if (!(a)) \
    {  puts("Assertion Failed ('" LS_STRINGIFY(a) "') in File '" __FILE__ "' (Line " LS_STRINGIFY(__LINE__) ")"); \
      __debugbreak(); \
  } } while (0)
#else
#define lsAssert(a) do { if (!(a)) { } } while (0)
#endif

template <typename T, typename U>
constexpr inline auto lsMax(const T &a, const U &b) -> decltype(a > b ? a : b)
{
  return a > b ? a : b;
}

template <typename T, typename U>
constexpr inline auto lsMin(const T &a, const U &b) -> decltype(a < b ? a : b)
{
  return a < b ? a : b;
}

template <typename T>
constexpr inline T lsClamp(const T &a, const T &min, const T &max)
{
  if (a < min)
    return min;

  if (a > max)
    return max;

  return a;
}

#define LS_ARRAYSIZE_C_STYLE(arrayName) (sizeof(arrayName) / sizeof(arrayName[0]))

#ifdef LS_FORCE_ARRAYSIZE_C_STYLE
#define LS_ARRAYSIZE(arrayName) LS_ARRAYSIZE_C_STYLE(arrayName)
#else
template <typename T, size_t TCount>
inline constexpr size_t LS_ARRAYSIZE(const T(&)[TCount]) { return TCount; }
#endif

//////////////////////////////////////////////////////////////////////////

template <typename T>
inline void lsZeroMemory(_Out_ T *pData, size_t count = 1)
{
  if (pData == nullptr)
    return;

  memset(pData, 0, sizeof(T) * count);
}

template <typename T>
inline void lsMemset(_Out_ T *pData, const size_t count, uint8_t data = 0)
{
  if (pData == nullptr)
    return;

  memset(pData, (int)data, sizeof(T) * count);
}

template <typename T>
inline void lsMemcpy(T *pDst, const T *pSrc, const size_t count)
{
  memcpy(pDst, pSrc, sizeof(T) * count);
}

template <typename T>
inline lsResult lsAlloc(_Out_ T **ppData, const size_t count = 1)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(ppData == nullptr, lsR_ArgumentNull);
  
  T *pData = reinterpret_cast<T *>(malloc(sizeof(T) * count));
  LS_ERROR_IF(pData == nullptr, lsR_MemoryAllocationFailure);

  *ppData = pData;

  goto epilogue;

epilogue:
  if (LS_FAILED(result))
    *ppData = nullptr;

  return result;
}

template <typename T>
inline lsResult lsAllocZero(_Out_ T **ppData, const size_t count = 1)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(lsAlloc(ppData, count));
  lsZeroMemory(*ppData, count);

epilogue:
  return result;
}

template <typename T>
inline lsResult lsRealloc(_In_ _Out_ T **ppData, const size_t newCount)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(ppData == nullptr, lsR_ArgumentNull);

  T *pData = reinterpret_cast<T *>(realloc(*ppData, sizeof(T) * newCount));
  LS_ERROR_IF(pData == nullptr, lsR_MemoryAllocationFailure);

  *ppData = pData;

  goto epilogue;

epilogue:
  if (LS_FAILED(result))
    *ppData = nullptr;

  return result;
}

template <typename T>
inline void lsFreePtr(_Out_ T **ppData)
{
  if (ppData != nullptr && *ppData != nullptr)
  {
    free((void *)(*ppData));
    *ppData = nullptr;
  }
}

//////////////////////////////////////////////////////////////////////////

#define lsPI M_PI
#define lsTWOPI 6.283185307179586476925286766559
#define lsHALFPI M_PI_2
#define lsQUARTERPI M_PI_4
#define lsSQRT2 M_SQRT2
#define lsINVSQRT2 M_SQRT1_2
#define lsSQRT3 1.414213562373095048801688724209698
#define lsINV_SQRT3 0.5773502691896257645091487805019574556
#define lsPIf 3.141592653589793f
#define lsTWOPIf 6.283185307179586f
#define lsHALFPIf ((float)M_PI_2)
#define lsQUARTERPIf ((float)M_PI_4)
#define lsSQRT2f 1.414213562373095f
#define lsINVSQRT2f 0.7071067811865475f
#define lsSQRT3f 1.414213562373095f
#define lsINVSQRT3f 0.57735026918962576f
#define lsEULER M_E
#define lsLOG2E M_LOG2E
#define lsLOG10E M_LOG10E
#define lsLN2 M_LN2
#define lsLN10 M_LN10
#define lsEULERf ((float)M_E)
#define lsLOG2Ef ((float)M_LOG2E)
#define lsLOG10Ef ((float)M_LOG10E)
#define lsLN2f ((float)M_LN2)
#define lsLN10f ((float)M_LN10)

#define lsDEG2RAD (lsPI / 180.0)
#define lsDEG2RADf (lsPIf / 180.0f)
#define lsRAD2DEG (180.0 / lsPI)
#define lsRAD2DEGf (180.0f / lsPIf)

//////////////////////////////////////////////////////////////////////////

template <typename T> constexpr inline T lsAbs(const T value) { return value >= 0 ? value : -value; }
template <> inline float_t lsAbs(const float_t value) { return fabsf(value); }
template <> inline double_t lsAbs(const double_t value) { return abs(value); }

inline double_t lsFloor(const double_t value) { return floor(value); }
inline float_t lsFloor(const float_t value) { return floorf(value); }
inline double_t lsCeil(const double_t value) { return ceil(value); }
inline float_t lsCeil(const float_t value) { return ceilf(value); }
inline double_t lsRound(const double_t value) { return round(value); }
inline float_t lsRound(const float_t value) { return roundf(value); }
inline double_t lsCopySign(const double_t value, const double_t sign) { return copysign(value, sign); }
inline float_t lsCopySign(const float_t value, const float_t sign) { return copysignf(value, sign); }

template <typename T> constexpr inline T lsMax(const T a, const T b) { return (a >= b) ? a : b; }
template <typename T> constexpr inline T lsMin(const T a, const T b) { return (a <= b) ? a : b; }

template <typename T, typename U>
constexpr inline auto lsLerp(const T a, const T b, const U ratio) -> decltype(a + (b - a) * ratio) { return a + (b - a) * ratio; }

template <typename T>
struct vec2t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[2];

    struct
    {
      T x, y;
    };
  };
#pragma warning(pop)

  inline vec2t() : x(0), y(0) {}
  inline explicit vec2t(T _v) : x(_v), y(_v) {}
  inline vec2t(T _x, T _y) : x(_x), y(_y) {}

  template <typename T2> inline explicit vec2t(const vec2t<T2> &cast) : x((T)cast.x), y((T)cast.y) { }

  inline vec2t<T>  operator +  (const vec2t<T> &a) const { return vec2t<T>(x + a.x, y + a.y); };
  inline vec2t<T>  operator -  (const vec2t<T> &a) const { return vec2t<T>(x - a.x, y - a.y); };
  inline vec2t<T>  operator *  (const vec2t<T> &a) const { return vec2t<T>(x * a.x, y * a.y); };
  inline vec2t<T>  operator /  (const vec2t<T> &a) const { return vec2t<T>(x / a.x, y / a.y); };
  inline vec2t<T> &operator += (const vec2t<T> &a) { return *this = vec2t<T>(x + a.x, y + a.y); };
  inline vec2t<T> &operator -= (const vec2t<T> &a) { return *this = vec2t<T>(x - a.x, y - a.y); };
  inline vec2t<T> &operator *= (const vec2t<T> &a) { return *this = vec2t<T>(x * a.x, y * a.y); };
  inline vec2t<T> &operator /= (const vec2t<T> &a) { return *this = vec2t<T>(x / a.x, y / a.y); };
  inline vec2t<T>  operator *  (const T a) const { return vec2t<T>(x * a, y * a); };
  inline vec2t<T>  operator /  (const T a) const { return vec2t<T>(x / a, y / a); };
  inline vec2t<T> &operator *= (const T a) { return *this = vec2t<T>(x * a, y * a); };
  inline vec2t<T> &operator /= (const T a) { return *this = vec2t<T>(x / a, y / a); };
  inline vec2t<T>  operator << (const T a) const { return vec2t<T>(x << a, y << a); };
  inline vec2t<T>  operator >> (const T a) const { return vec2t<T>(x >> a, y >> a); };
  inline vec2t<T> &operator <<= (const T a) { return *this = vec2t<T>(x << a, y << a); };
  inline vec2t<T> &operator >>= (const T a) { return *this = vec2t<T>(x >> a, y >> a); };

  inline vec2t<T>  operator -  () const { return vec2t<T>(-x, -y); };

  inline bool       operator == (const vec2t<T> &a) const { return x == a.x && y == a.y; };
  inline bool       operator != (const vec2t<T> &a) const { return x != a.x || y != a.y; };

  inline float_t Length() const { return sqrtf(x * x + y * y); };
  inline T LengthSquared() const { return x * x + y * y; };
  inline vec2t<T> Normalize() const { return *this / (T)Length(); };

  inline typename float_t AspectRatio() const { return (float_t)x / (float_t)y; };
  inline typename float_t InverseAspectRatio() const { return (float_t)y / (float_t)x; };

  inline static T Dot(const vec2t<T> a, const vec2t<T> b)
  {
    return a.x * b.x + a.y * b.y;
  };
};

template <typename T>
inline vec2t<T>  operator *  (const T a, const vec2t<T> b) { return vec2t<T>(a * b.x, a * b.y); };

template <typename T>
inline vec2t<T>  operator /  (const T a, const vec2t<T> b) { return vec2t<T>(a / b.x, a / b.y); };

template <typename T> T lsMax(const vec2t<T> &v) { return lsMax(v.x, v.y); }
template <typename T> T lsMin(const vec2t<T> &v) { return lsMin(v.x, v.y); }

template <typename T> vec2t<T> lsMax(const vec2t<T> &a, const vec2t<T> &b) { return vec2t<T>(fpMax(a.x, b.x), lsMax(a.y, b.y)); }
template <typename T> vec2t<T> lsMin(const vec2t<T> &a, const vec2t<T> &b) { return vec2t<T>(fpMin(a.x, b.x), lsMin(a.y, b.y)); }

template <typename T> vec2t<T> lsAbs(const vec2t<T> &a) { return vec2t<T>(fpAbs(a.x), lsAbs(a.y)); }
template <typename T> vec2t<T> lsFloor(const vec2t<T> &a) { return vec2t<T>(fpFloor(a.x), lsFloor(a.y)); }
template <typename T> vec2t<T> lsCeil(const vec2t<T> &a) { return vec2t<T>(fpCeil(a.x), lsCeil(a.y)); }
template <typename T> vec2t<T> lsRound(const vec2t<T> &a) { return vec2t<T>(fpRound(a.x), lsRound(a.y)); }

typedef vec2t<size_t> vec2s;
typedef vec2t<int64_t> vec2i;
typedef vec2t<uint64_t> vec2u;
typedef vec2t<int32_t> vec2i32;
typedef vec2t<uint32_t> vec2u32;
typedef vec2t<float_t> vec2f;
typedef vec2t<double_t> vec2d;

//////////////////////////////////////////////////////////////////////////


template <typename T>
struct vec3t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[3];

    struct
    {
      T x, y, z;
    };
  };
#pragma warning(pop)

  constexpr inline vec3t() : x(0), y(0), z(0) {}
  constexpr inline explicit vec3t(T _v) : x(_v), y(_v), z(_v) {}

  // Cartesian: x, y, z;
  // Spherical: radius, theta, phi;
  // Cylindrical: rho, phi, z;
  constexpr inline vec3t(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  constexpr inline explicit vec3t(vec2t<T> vector2, T _z) : x(vector2.x), y(vector2.y), z(_z) {}

  template <typename T2> constexpr inline explicit vec3t(const vec3t<T2> &cast) : x((T)cast.x), y((T)cast.y), z((T)cast.z) {}

  constexpr inline vec3t<T>  operator +  (const vec3t<T> &a) const { return vec3t<T>(x + a.x, y + a.y, z + a.z); };
  constexpr inline vec3t<T>  operator -  (const vec3t<T> &a) const { return vec3t<T>(x - a.x, y - a.y, z - a.z); };
  constexpr inline vec3t<T>  operator *  (const vec3t<T> &a) const { return vec3t<T>(x * a.x, y * a.y, z * a.z); };
  constexpr inline vec3t<T>  operator /  (const vec3t<T> &a) const { return vec3t<T>(x / a.x, y / a.y, z / a.z); };
  constexpr inline vec3t<T> &operator += (const vec3t<T> &a) { return *this = vec3t<T>(x + a.x, y + a.y, z + a.z); };
  constexpr inline vec3t<T> &operator -= (const vec3t<T> &a) { return *this = vec3t<T>(x - a.x, y - a.y, z - a.z); };
  constexpr inline vec3t<T> &operator *= (const vec3t<T> &a) { return *this = vec3t<T>(x * a.x, y * a.y, z * a.z); };
  constexpr inline vec3t<T> &operator /= (const vec3t<T> &a) { return *this = vec3t<T>(x / a.x, y / a.y, z / a.z); };
  constexpr inline vec3t<T>  operator *  (const T a) const { return vec3t<T>(x * a, y * a, z * a); };
  constexpr inline vec3t<T>  operator /  (const T a) const { return vec3t<T>(x / a, y / a, z / a); };
  constexpr inline vec3t<T> &operator *= (const T a) { return *this = vec3t<T>(x * a, y * a, z * a); };
  constexpr inline vec3t<T> &operator /= (const T a) { return *this = vec3t<T>(x / a, y / a, z / a); };

  constexpr inline vec3t<T>  operator -  () const { return vec3t<T>(-x, -y, -z); };

  constexpr inline bool       operator == (const vec3t<T> &a) const { return x == a.x && y == a.y && z == a.z; };
  constexpr inline bool       operator != (const vec3t<T> &a) const { return x != a.x || y != a.y || z != a.z; };

  inline typename float_t Length() const { return sqrtf(x * x + y * y + z * z); };
  constexpr inline T LengthSquared() const { return x * x + y * y + z * z; };
  inline vec3t<T> Normalize() const { return *this / (T)Length(); };

  constexpr inline vec2t<T> ToVector2() const { return vec2t<T>(x, y); };

  constexpr inline static vec3t<T> Cross(const vec3t<T> a, const vec3t<T> b)
  {
    return vec3t<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
  };

  constexpr inline static T Dot(const vec3t<T> a, const vec3t<T> b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  };
};

template <typename T>
constexpr inline vec3t<T>  operator *  (const T a, const vec3t<T> b) { return vec3t<T>(a * b.x, a * b.y, a * b.z); };

template <typename T>
constexpr inline vec3t<T>  operator /  (const T a, const vec3t<T> b) { return vec3t<T>(a / b.x, a / b.y, a / b.z); };

template <typename T> constexpr inline T lsMax(const vec3t<T> &v) { return lsMax(lsMax(v.x, v.y), v.z); }
template <typename T> constexpr inline T lsMin(const vec3t<T> &v) { return lsMin(lsMin(v.x, v.y), v.z); }

template <typename T> constexpr inline vec3t<T> lsMax(const vec3t<T> &a, const vec3t<T> &b) { return vec3t<T>(lsMax(a.x, b.x), lsMax(a.y, b.y), lsMax(a.z, b.z)); }
template <typename T> constexpr inline vec3t<T> lsMin(const vec3t<T> &a, const vec3t<T> &b) { return vec3t<T>(lsMin(a.x, b.x), lsMin(a.y, b.y), lsMin(a.z, b.z)); }

template <typename T> inline vec3t<T> lsAbs(const vec3t<T> &a) { return vec3t<T>(lsAbs(a.x), lsAbs(a.y), lsAbs(a.z)); }
template <typename T> inline vec3t<T> lsFloor(const vec3t<T> &a) { return vec3t<T>(lsFloor(a.x), lsFloor(a.y), lsFloor(a.z)); }
template <typename T> inline vec3t<T> lsCeil(const vec3t<T> &a) { return vec3t<T>(lsCeil(a.x), lsCeil(a.y), lsCeil(a.z)); }
template <typename T> inline vec3t<T> lsRound(const vec3t<T> &a) { return vec3t<T>(lsRound(a.x), lsRound(a.y), lsRound(a.z)); }

typedef vec3t<size_t> vec3s;
typedef vec3t<int64_t> vec3i;
typedef vec3t<uint64_t> vec3u;
typedef vec3t<int32_t> vec3i32;
typedef vec3t<uint32_t> vec3u32;
typedef vec3t<float_t> vec3f;
typedef vec3t<double_t> vec3d;

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct vec4t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[4];

    struct
    {
      T x, y, z, w;
    };
  };
#pragma warning(pop)

  constexpr inline vec4t() : x(0), y(0), z(0), w(0) {}
  constexpr inline explicit vec4t(T _v) : x(_v), y(_v), z(_v), w(_v) {}
  constexpr inline vec4t(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
  constexpr inline explicit vec4t(const vec3t<T> vec3, const T _w) : x(vec3.x), y(vec3.y), z(vec3.z), w(_w) {}
  constexpr inline explicit vec4t(const T _x, const vec3t<T> vec3) : x(_x), y(vec3.x), z(vec3.y), w(vec3.z) {}
  constexpr inline explicit vec4t(const vec2t<T> vec2, const T _z, const T _w) : x(vec2.x), y(vec2.y), z(_z), w(_w) {}
  constexpr inline explicit vec4t(const vec2t<T> vec2a, const vec2t<T> vec2b) : x(vec2a.x), y(vec2a.y), z(vec2b.x), w(vec2b.y) {}

  template <typename T2> constexpr inline explicit vec4t(const vec4t<T2> &cast) : x((T)cast.x), y((T)cast.y), z((T)cast.z), w((T)cast.w) {}

  constexpr inline vec4t<T>  operator +  (const vec4t<T> &a) const { return vec4t<T>(x + a.x, y + a.y, z + a.z, w + a.w); };
  constexpr inline vec4t<T>  operator -  (const vec4t<T> &a) const { return vec4t<T>(x - a.x, y - a.y, z - a.z, w - a.w); };
  constexpr inline vec4t<T>  operator *  (const vec4t<T> &a) const { return vec4t<T>(x * a.x, y * a.y, z * a.z, w * a.w); };
  constexpr inline vec4t<T>  operator /  (const vec4t<T> &a) const { return vec4t<T>(x / a.x, y / a.y, z / a.z, w / a.w); };
  constexpr inline vec4t<T> &operator += (const vec4t<T> &a) { return *this = vec4t<T>(x + a.x, y + a.y, z + a.z, w + a.w); };
  constexpr inline vec4t<T> &operator -= (const vec4t<T> &a) { return *this = vec4t<T>(x - a.x, y - a.y, z - a.z, w - a.w); };
  constexpr inline vec4t<T> &operator *= (const vec4t<T> &a) { return *this = vec4t<T>(x * a.x, y * a.y, z * a.z, w * a.w); };
  constexpr inline vec4t<T> &operator /= (const vec4t<T> &a) { return *this = vec4t<T>(x / a.x, y / a.y, z / a.z, w / a.w); };
  constexpr inline vec4t<T>  operator *  (const T a) const { return vec4t<T>(x * a, y * a, z * a, w * a); };
  constexpr inline vec4t<T>  operator /  (const T a) const { return vec4t<T>(x / a, y / a, z / a, w / a); };
  constexpr inline vec4t<T> &operator *= (const T a) { return *this = vec4t<T>(x * a, y * a, z * a, w * a); };
  constexpr inline vec4t<T> &operator /= (const T a) { return *this = vec4t<T>(x / a, y / a, z / a, w / a); };

  constexpr inline vec4t<T>  operator -  () const { return vec4t<T>(-x, -y, -z, -w); };

  constexpr inline bool       operator == (const vec4t<T> &a) const { return x == a.x && y == a.y && z == a.z && w == a.w; };
  constexpr inline bool       operator != (const vec4t<T> &a) const { return x != a.x || y != a.y || z != a.z || w != a.w; };

  inline typename float_t Length() const { return sqrtf(x * x + y * y + z * z + w * w); };
  constexpr inline T LengthSquared() const { return x * x + y * y + z * z + w * w; };
  inline vec4t<T> Normalize() const { return *this / (T)Length(); };

  constexpr inline vec2t<T> ToVector2() const { return vec2t<T>(x, y); };
  constexpr inline vec3t<T> ToVector3() const { return vec3t<T>(x, y, z); };

  constexpr inline static T Dot(const vec4t<T> a, const vec4t<T> b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  };

};

template <typename T>
constexpr inline vec4t<T>  operator *  (const T a, const vec4t<T> b) { return vec4t<T>(a * b.x, a * b.y, a * b.z, a * b.w); };

template <typename T>
constexpr inline vec4t<T>  operator /  (const T a, const vec4t<T> b) { return vec4t<T>(a / b.x, a / b.y, a / b.z, a / b.w); };

template <typename T> constexpr T lsMax(const vec4t<T> &v) { return lsMax(lsMax(v.x, v.y), lsMax(v.z, v.w)); }
template <typename T> constexpr T lsMin(const vec4t<T> &v) { return lsMin(lsMin(v.x, v.y), lsMin(v.z, v.w)); }

template <typename T> constexpr vec4t<T> lsMax(const vec4t<T> &a, const vec4t<T> &b) { return vec4t<T>(lsMax(a.x, b.x), lsMax(a.y, b.y), lsMax(a.z, b.z), lsMax(a.w, b.w)); }
template <typename T> constexpr vec4t<T> lsMin(const vec4t<T> &a, const vec4t<T> &b) { return vec4t<T>(lsMin(a.x, b.x), lsMin(a.y, b.y), lsMin(a.z, b.z), lsMin(a.w, b.w)); }

template <typename T> vec4t<T> lsAbs(const vec4t<T> &a) { return vec4t<T>(lsAbs(a.x), lsAbs(a.y), lsAbs(a.z), lsAbs(a.w)); }
template <typename T> vec4t<T> lsFloor(const vec4t<T> &a) { return vec4t<T>(lsFloor(a.x), lsFloor(a.y), lsFloor(a.z), lsFloor(a.w)); }
template <typename T> vec4t<T> lsCeil(const vec4t<T> &a) { return vec4t<T>(lsCeil(a.x), lsCeil(a.y), lsCeil(a.z), lsCeil(a.w)); }
template <typename T> vec4t<T> lsRound(const vec4t<T> &a) { return vec4t<T>(lsRound(a.x), lsRound(a.y), lsRound(a.z), lsRound(a.w)); }

typedef vec4t<size_t> vec4s;
typedef vec4t<int64_t> vec4i;
typedef vec4t<uint64_t> vec4u;
typedef vec4t<int32_t> vec4i32;
typedef vec4t<uint32_t> vec4u32;
typedef vec4t<float_t> vec4f;
typedef vec4t<double_t> vec4d;

//////////////////////////////////////////////////////////////////////////

int64_t lsGetCurrentTimeMs();
int64_t lsGetCurrentTimeNs();
uint64_t lsGetRand();

//////////////////////////////////////////////////////////////////////////

int64_t lsParseInt(_In_ const char *start, _Out_ const char **pEnd = nullptr);
uint64_t lsParseUInt(_In_ const char *start, _Out_ const char **pEnd = nullptr);
double_t lsParseFloat(_In_ const char *start, _Out_ const char **pEnd = nullptr);

bool lsIsInt(_In_ const char *text);
bool lsIsInt(_In_ const char *text, const size_t length);
bool lsIsUInt(_In_ const char *text);
bool lsIsUInt(_In_ const char *text, const size_t length);
bool lsIsFloat(_In_ const char *text);
bool lsIsFloat(_In_ const char *text, const size_t length);

bool lsStartsWithInt(_In_ const char *text);
bool lsStartsWithInt(_In_ const char *text, const size_t length);
bool lsStartsWithUInt(_In_ const char *text);
bool lsStartsWithUInt(_In_ const char *text, const size_t length);

int64_t lsParseInt(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd = nullptr);
uint64_t lsParseUInt(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd = nullptr);
double_t lsParseFloat(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd = nullptr);

bool lsIsInt(_In_ const wchar_t *text);
bool lsIsInt(_In_ const wchar_t *text, const size_t length);
bool lsIsUInt(_In_ const wchar_t *text);
bool lsIsUInt(_In_ const wchar_t *text, const size_t length);
bool lsIsFloat(_In_ const wchar_t *text);
bool lsIsFloat(_In_ const wchar_t *text, const size_t length);

bool lsStartsWithInt(_In_ const wchar_t *text);
bool lsStartsWithInt(_In_ const wchar_t *text, const size_t length);
bool lsStartsWithUInt(_In_ const wchar_t *text);
bool lsStartsWithUInt(_In_ const wchar_t *text, const size_t length);
