#include "core.h"

int64_t lsGetCurrentTimeMs()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int64_t lsGetCurrentTimeNs()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

uint64_t lsGetRand()
{
  __declspec(align(16)) static uint64_t last[2] = { (uint64_t)lsGetCurrentTimeNs(), __rdtsc() };
  __declspec(align(16)) static uint64_t last2[2] = { ~__rdtsc(), ~(uint64_t)lsGetCurrentTimeNs() };

  const __m128i a = _mm_load_si128(reinterpret_cast<__m128i *>(last));
  const __m128i b = _mm_load_si128(reinterpret_cast<__m128i *>(last2));

  const __m128i r = _mm_aesdec_si128(a, b);

  _mm_store_si128(reinterpret_cast<__m128i *>(last), b);
  _mm_store_si128(reinterpret_cast<__m128i *>(last2), r);

  return last[1] ^ last[0];

  //static uint64_t last[2] = { (uint64_t)GetCurrentTimeNs(), __rdtsc() };
  //
  //const uint64_t oldstate_hi = last[0];
  //const uint64_t oldstate_lo = oldstate_hi * 6364136223846793005ULL + (last[1] | 1);
  //last[0] = oldstate_hi * 6364136223846793005ULL + (last[1] | 1);
  //
  //const uint32_t xorshifted_hi = (uint32_t)(((oldstate_hi >> 18) ^ oldstate_hi) >> 27);
  //const uint32_t rot_hi = (uint32_t)(oldstate_hi >> 59);
  //
  //const uint32_t xorshifted_lo = (uint32_t)(((oldstate_lo >> 18) ^ oldstate_lo) >> 27);
  //const uint32_t rot_lo = (uint32_t)(oldstate_lo >> 59);
  //
  //const uint32_t hi = (xorshifted_hi >> rot_hi) | (xorshifted_hi << (uint32_t)((-(int32_t)rot_hi) & 31));
  //const uint32_t lo = (xorshifted_lo >> rot_lo) | (xorshifted_lo << (uint32_t)((-(int32_t)rot_lo) & 31));
  //
  //return ((uint64_t)hi << 32) | lo;
}

//////////////////////////////////////////////////////////////////////////

int64_t lsParseInt(_In_ const char *start, _Out_ const char **pEnd /* = nullptr */)
{
  const char *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  int64_t ret = 0;

  // See: https://graphics.stanford.edu/~seander/bithacks.html#ConditionalNegate
  int64_t negate = 0;

  if (*start == '-')
  {
    negate = 1;
    start++;
  }

  while (true)
  {
    uint8_t digit = *start - '0';

    if (digit > 9)
      break;

    ret = (ret << 1) + (ret << 3) + digit;
    start++;
  }

  *pEnd = start;

  return (ret ^ -negate) + negate;
}

uint64_t lsParseUInt(_In_ const char *start, _Out_ const char **pEnd /* = nullptr */)
{
  const char *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  uint64_t ret = 0;

  while (true)
  {
    uint8_t digit = *start - '0';

    if (digit > 9)
      break;

    ret = (ret << 1) + (ret << 3) + digit;
    start++;
  }

  *pEnd = start;

  return ret;
}

double_t lsParseFloat(_In_ const char *start, _Out_ const char **pEnd /* = nullptr */)
{
  const char *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  uint64_t sign = 0;

  if (*start == '-')
  {
    sign = (uint64_t)1 << 63; // IEEE floating point signed bit.
    ++start;
  }

  const char *_end = start;
  const int64_t left = lsParseInt(start, &_end);
  double_t ret = (double_t)left;

  if (*_end == '.')
  {
    start = _end + 1;
    const int64_t right = lsParseInt(start, &_end);

    const double_t fracMult[] = { 0.0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11, 1e-12, 1e-13 };

    if (_end - start < (ptrdiff_t)LS_ARRAYSIZE(fracMult))
      ret = (ret + right * fracMult[_end - start]);
    else
      ret = (ret + right * pow(10, _end - start));

    // Get Sign. (memcpy should get optimized away and is only there to prevent undefined behavior)
    {
      uint64_t data;
      static_assert(sizeof(data) == sizeof(ret), "Platform not supported.");
      memcpy(&data, &ret, sizeof(data));
      data ^= sign;
      memcpy(&ret, &data, sizeof(data));
    }

    *pEnd = _end;

    if (*_end == 'e' || *_end == 'E')
    {
      start = ++_end;

      if ((*start >= '0' && *start <= '9') || *start == '-')
      {
        ret *= pow(10, lsParseInt(start, &_end));

        *pEnd = _end;
      }
    }
  }
  else
  {
    // Get Sign. (memcpy should get optimized away and is only there to prevent undefined behavior)
    {
      uint64_t data;
      static_assert(sizeof(data) == sizeof(ret), "Platform not supported.");
      memcpy(&data, &ret, sizeof(data));
      data ^= sign;
      memcpy(&ret, &data, sizeof(data));
    }

    if (*_end == 'e' || *_end == 'E')
    {
      start = ++_end;

      if ((*start >= '0' && *start <= '9') || *start == '-')
        ret *= pow(10, lsParseInt(start, &_end));
    }

    *pEnd = _end;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsInt(_In_ const char *text)
{
  if (text == nullptr)
    return false;

  return lsIsInt(text, strlen(text));
}

bool lsIsInt(_In_ const char *text, const size_t length)
{
  if (text == nullptr)
    return false;

  const bool sign = (text[0] == '-');
  size_t i = (size_t)sign;

  for (; i < length; i++)
  {
    if (text[i] < '0' || text[i] > '9')
    {
      if (text[i] == '\0')
        return (i > (size_t)sign);

      return false;
    }
  }

  return i > (size_t)sign;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsUInt(_In_ const char *text)
{
  if (text == nullptr)
    return false;

  return lsIsUInt(text, strlen(text));
}

bool lsIsUInt(_In_ const char *text, const size_t length)
{
  if (text == nullptr)
    return false;

  size_t i = 0;

  for (; i < length; i++)
  {
    if (text[i] < '0' || text[i] > '9')
    {
      if (text[i] == '\0')
        return i > 0;

      return false;
    }
  }

  return i > 0;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsFloat(_In_ const char *text)
{
  if (text == nullptr)
    return false;

  return lsIsFloat(text, strlen(text));
}

bool lsIsFloat(_In_ const char *text, const size_t length)
{
  if (text == nullptr)
    return false;

  bool hasDigits = false;
  bool hasPostPeriodDigits = false;
  bool hasPostExponentDigits = false;

  size_t i = (size_t)(text[0] == '-');

  for (; i < length; i++)
  {
    if (text[i] == '\0')
    {
      return hasDigits;
    }
    else if (text[i] == '.')
    {
      i++;
      goto period;
    }
    else if (text[i] == 'e' || text[i] == 'E')
    {
      if (!hasDigits)
        return false;

      i++;
      goto exponent;
    }
    else if (text[i] < '0' || text[i] > '9')
    {
      return false;
    }
    else
    {
      hasDigits = true;
    }
  }

  return hasDigits;

period:
  for (; i < length; i++)
  {
    if (text[i] == '\0')
    {
      return hasDigits || hasPostPeriodDigits;
    }
    else if (text[i] == 'e' || text[i] == 'E')
    {
      if (!(hasDigits || hasPostPeriodDigits))
        return false;

      i++;
      goto exponent;
    }
    else if (text[i] < '0' || text[i] > '9')
    {
      return false;
    }
    else
    {
      hasPostPeriodDigits = true;
    }
  }

  return hasDigits || hasPostPeriodDigits;

exponent:
  i += (size_t)(text[i] == '-');

  for (; i < length; i++)
  {
    if (text[i] < '0' || text[i] > '9')
    {
      if (text[i] == '\0')
        return hasPostExponentDigits && (hasPostPeriodDigits || hasDigits);

      return false;
    }
    else
    {
      hasPostExponentDigits = true;
    }
  }

  return hasPostExponentDigits && (hasPostPeriodDigits || hasDigits);
}

//////////////////////////////////////////////////////////////////////////

bool lsStartsWithInt(_In_ const char *text)
{
  if (text == nullptr)
    return false;

  return lsStartsWithInt(text, strlen(text));
}

bool lsStartsWithInt(_In_ const char *text, const size_t length)
{
  if (text == nullptr)
    return false;

  const bool sign = (text[0] == '-');
  size_t i = (size_t)sign;

  for (; i < length; i++)
    if (text[i] < '0' || text[i] > '9')
      return (i > (size_t)sign);

  return i > (size_t)sign;
}

//////////////////////////////////////////////////////////////////////////

bool lsStartsWithUInt(_In_ const char *text)
{
  if (text == nullptr)
    return false;

  return lsStartsWithUInt(text, strlen(text));
}

bool lsStartsWithUInt(_In_ const char *text, const size_t length)
{
  if (text == nullptr)
    return false;

  size_t i = 0;

  for (; i < length; i++)
    if (text[i] < '0' || text[i] > '9')
      return i > 0;

  return i > 0;
}

//////////////////////////////////////////////////////////////////////////

int64_t lsParseInt(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd /* = nullptr */)
{
  const wchar_t *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  int64_t ret = 0;

  // See: https://graphics.stanford.edu/~seander/bithacks.html#ConditionalNegate
  int64_t negate = 0;

  if (*start == L'-')
  {
    negate = 1;
    start++;
  }

  while (true)
  {
    uint16_t digit = *start - L'0';

    if (digit > 9)
      break;

    ret = (ret << 1) + (ret << 3) + digit;
    start++;
  }

  *pEnd = start;

  return (ret ^ -negate) + negate;
}

uint64_t lsParseUInt(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd /* = nullptr */)
{
  const wchar_t *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  uint64_t ret = 0;

  while (true)
  {
    uint16_t digit = *start - L'0';

    if (digit > 9)
      break;

    ret = (ret << 1) + (ret << 3) + digit;
    start++;
  }

  *pEnd = start;

  return ret;
}

double_t lsParseFloat(_In_ const wchar_t *start, _Out_ const wchar_t **pEnd /* = nullptr */)
{
  const wchar_t *endIfNoEnd = nullptr;

  if (pEnd == nullptr)
    pEnd = &endIfNoEnd;

  uint64_t sign = 0;

  if (*start == L'-')
  {
    sign = (uint64_t)1 << 63; // IEEE floating point signed bit.
    ++start;
  }

  const wchar_t *_end = start;
  const int64_t left = lsParseInt(start, &_end);
  double_t ret = (double_t)left;

  if (*_end == L'.')
  {
    start = _end + 1;
    const int64_t right = lsParseInt(start, &_end);

    const double_t fracMult[] = { 0.0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11, 1e-12, 1e-13 };

    if (_end - start < (ptrdiff_t)LS_ARRAYSIZE(fracMult))
      ret = (ret + right * fracMult[_end - start]);
    else
      ret = (ret + right * pow(10, _end - start));

    // Get Sign. (memcpy should get optimized away and is only there to prevent undefined behavior)
    {
      uint64_t data;
      static_assert(sizeof(data) == sizeof(ret), "Platform not supported.");
      memcpy(&data, &ret, sizeof(data));
      data ^= sign;
      memcpy(&ret, &data, sizeof(data));
    }

    *pEnd = _end;

    if (*_end == L'e' || *_end == L'E')
    {
      start = ++_end;

      if ((*start >= L'0' && *start <= L'9') || *start == L'-')
      {
        ret *= pow(10, lsParseInt(start, &_end));

        *pEnd = _end;
      }
    }
  }
  else
  {
    // Get Sign. (memcpy should get optimized away and is only there to prevent undefined behavior)
    {
      uint64_t data;
      static_assert(sizeof(data) == sizeof(ret), "Platform not supported.");
      memcpy(&data, &ret, sizeof(data));
      data ^= sign;
      memcpy(&ret, &data, sizeof(data));
    }

    if (*_end == L'e' || *_end == L'E')
    {
      start = ++_end;

      if ((*start >= L'0' && *start <= L'9') || *start == L'-')
        ret *= pow(10, lsParseInt(start, &_end));
    }

    *pEnd = _end;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsInt(_In_ const wchar_t *text)
{
  if (text == nullptr)
    return false;

  return lsIsInt(text, wcslen(text));
}

bool lsIsInt(_In_ const wchar_t *text, const size_t length)
{
  if (text == nullptr)
    return false;

  const bool sign = (text[0] == L'-');
  size_t i = (size_t)sign;

  for (; i < length; i++)
  {
    if (text[i] < L'0' || text[i] > L'9')
    {
      if (text[i] == L'\0')
        return (i > (size_t)sign);

      return false;
    }
  }

  return i > (size_t)sign;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsUInt(_In_ const wchar_t *text)
{
  if (text == nullptr)
    return false;

  return lsIsUInt(text, wcslen(text));
}

bool lsIsUInt(_In_ const wchar_t *text, const size_t length)
{
  if (text == nullptr)
    return false;

  size_t i = 0;

  for (; i < length; i++)
  {
    if (text[i] < L'0' || text[i] > L'9')
    {
      if (text[i] == L'\0')
        return i > 0;

      return false;
    }
  }

  return i > 0;
}

//////////////////////////////////////////////////////////////////////////

bool lsIsFloat(_In_ const wchar_t *text)
{
  if (text == nullptr)
    return false;

  return lsIsFloat(text, wcslen(text));
}

bool lsIsFloat(_In_ const wchar_t *text, const size_t length)
{
  if (text == nullptr)
    return false;

  bool hasDigits = false;
  bool hasPostPeriodDigits = false;
  bool hasPostExponentDigits = false;

  size_t i = (size_t)(text[0] == L'-');

  for (; i < length; i++)
  {
    if (text[i] == L'\0')
    {
      return hasDigits;
    }
    else if (text[i] == L'.')
    {
      i++;
      goto period;
    }
    else if (text[i] == L'e' || text[i] == L'E')
    {
      if (!hasDigits)
        return false;

      i++;
      goto exponent;
    }
    else if (text[i] < L'0' || text[i] > L'9')
    {
      return false;
    }
    else
    {
      hasDigits = true;
    }
  }

  return hasDigits;

period:
  for (; i < length; i++)
  {
    if (text[i] == L'\0')
    {
      return hasDigits || hasPostPeriodDigits;
    }
    else if (text[i] == L'e' || text[i] == L'E')
    {
      if (!(hasDigits || hasPostPeriodDigits))
        return false;

      i++;
      goto exponent;
    }
    else if (text[i] < L'0' || text[i] > L'9')
    {
      return false;
    }
    else
    {
      hasPostPeriodDigits = true;
    }
  }

  return hasDigits || hasPostPeriodDigits;

exponent:
  i += (size_t)(text[i] == L'-');

  for (; i < length; i++)
  {
    if (text[i] < L'0' || text[i] > L'9')
    {
      if (text[i] == L'\0')
        return hasPostExponentDigits && (hasPostPeriodDigits || hasDigits);

      return false;
    }
    else
    {
      hasPostExponentDigits = true;
    }
  }

  return hasPostExponentDigits && (hasPostPeriodDigits || hasDigits);
}

//////////////////////////////////////////////////////////////////////////

bool lsStartsWithInt(_In_ const wchar_t *text)
{
  if (text == nullptr)
    return false;

  return lsStartsWithInt(text, wcslen(text));
}

bool lsStartsWithInt(_In_ const wchar_t *text, const size_t length)
{
  if (text == nullptr)
    return false;

  const bool sign = (text[0] == L'-');
  size_t i = (size_t)sign;

  for (; i < length; i++)
    if (text[i] < L'0' || text[i] > L'9')
      return (i > (size_t)sign);

  return i > (size_t)sign;
}

//////////////////////////////////////////////////////////////////////////

bool lsStartsWithUInt(_In_ const wchar_t *text)
{
  if (text == nullptr)
    return false;

  return lsStartsWithUInt(text, wcslen(text));
}

bool lsStartsWithUInt(_In_ const wchar_t *text, const size_t length)
{
  if (text == nullptr)
    return false;

  size_t i = 0;

  for (; i < length; i++)
    if (text[i] < L'0' || text[i] > L'9')
      return i > 0;

  return i > 0;
}
