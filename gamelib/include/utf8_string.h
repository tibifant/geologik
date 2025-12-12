#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

inline bool lsCopyString(char *dst, const size_t dstSize, const char *src, const size_t srcSize)
{
  const size_t max = lsMin(dstSize, srcSize);

  for (size_t i = 0; i < max; i++)
  {
    dst[i] = src[i];

    if (src[i] == '\0')
      return true;
  }

  if (max)
    dst[max - 1] = '\0';

  return false;
}

inline bool lsCopyString(wchar_t *dst, const size_t dstCount, const wchar_t *src, const size_t srcCount)
{
  const size_t max = lsMin(dstCount, srcCount);

  for (size_t i = 0; i < max; i++)
  {
    dst[i] = src[i];

    if (src[i] == '\0')
      return true;
  }

  if (max)
    dst[max - 1] = '\0';

  return false;
}

template <size_t DstSize, size_t SrcSize>
bool lsCopyString(char(&dst)[DstSize], const char(&src)[SrcSize])
{
  return lsCopyString(dst, DstSize, src, SrcSize);
}

template <size_t DstSize>
bool lsCopyString(char(&dst)[DstSize], const char *src, const size_t srcSize)
{
  return lsCopyString(dst, DstSize, src, srcSize);
}

template <size_t DstSize>
bool lsCopyString(char(&dst)[DstSize], const char *src)
{
  return lsCopyString(dst, DstSize, src, (size_t)-1);
}

template <size_t DstSize, size_t SrcSize>
bool lsCopyString(wchar_t(&dst)[DstSize], const wchar_t(&src)[SrcSize])
{
  return lsCopyString(dst, DstSize, src, SrcSize);
}

template <size_t DstSize>
bool lsCopyString(wchar_t(&dst)[DstSize], const wchar_t *src, const size_t srcSize)
{
  return lsCopyString(dst, DstSize, src, srcSize);
}

template <size_t DstSize>
bool lsCopyString(wchar_t(&dst)[DstSize], const wchar_t *src)
{
  return lsCopyString(dst, DstSize, src, (size_t)-1);
}

inline bool lsStringEquals(const char *a, const size_t aSize, const char *b, const size_t bSize)
{
  return strncmp(a, b, lsMin(aSize, bSize)) == 0;
}

template <size_t ASize, size_t BSize>
bool lsStringEquals(const char(&a)[ASize], const char(&b)[BSize])
{
  return lsStringEquals(a, ASize, b, BSize);
}

template <size_t ASize, typename T>
std::enable_if_t<std::is_same<char *, T>::value || std::is_same<const char *, T>::value, bool> lsStringEquals(const char(&a)[ASize], T b)
{
  return lsStringEquals(a, ASize, b, ASize);
}

template <size_t ASize>
bool lsStringEquals(const char(&a)[ASize], const char *b, const size_t bSize)
{
  return lsStringEquals(a, ASize, b, bSize);
}

inline size_t lsStringLength(const char *text, const size_t maxCount)
{
  if (text == nullptr)
    return 0;

  return strnlen_s(text, maxCount);
}

template <size_t Size>
size_t lsStringLength(const char(&text)[Size])
{
  return lsStringLength(text, Size);
}

template <typename T>
std::enable_if_t<std::is_same<char *, T>::value || std::is_same<const char *, T>::value, size_t> lsStringLength(T text)
{
  return strlen(text);
}

//////////////////////////////////////////////////////////////////////////

constexpr size_t string_MaxUtf16CharInUtf8Chars = 3;

typedef int32_t utf32_t;

template <size_t TCount>
utf32_t lsToChar(const char c[TCount]);
utf32_t lsToChar(_In_ const char *c, const size_t size);

bool string_IsValidChar(const char *c, const size_t size, _Out_opt_ utf32_t *pChar = nullptr, _Out_opt_ size_t *pCharSize = nullptr);

struct iterated_char
{
  const char *character;
  utf32_t codePoint;
  size_t characterSize;
  size_t index;
  size_t offset;

  iterated_char(const char *character, const utf32_t codePoint, const size_t characterSize, const size_t index, const size_t offset);
};

struct utf8_string_iterator
{
  const char *string;
  size_t bytes;
  size_t charCount = 0;
  utf8::utf8_iter iter;

  utf8_string_iterator(const char *string);
  utf8_string_iterator(const char *string, size_t bytes);

  utf8_string_iterator &begin();
  utf8_string_iterator end();

  bool operator!=(const utf8_string_iterator &b);
  utf8_string_iterator &operator++();

  iterated_char operator*();
};

template <size_t TCount>
struct small_string;

struct string
{
  char *text = nullptr;

  // Includes null terminator.
  size_t bytes = 0;

  // Includes null terminator.
  size_t count = 0;

  size_t capacity = 0;
  bool hasFailed = false;

  string();

  template <size_t TSize>
  string(const char text[TSize]);

  string(_In_ const char *text, const size_t size);
  string(_In_ const char *text);

  template <size_t TSize>
  string(const wchar_t text[TSize]);

  string(const wchar_t *text, const size_t size);
  string(const wchar_t *text);

  ~string();

  string(const string &copy);
  string(string &&move);

  string &operator = (const string &copy);
  string &operator = (string &&move);

  // Size in bytes.
  // Includes null terminator.
  size_t Size() const;

  // Number of utf8 characters in this string.
  // Includes null terminator.
  size_t Count() const;

  utf32_t operator[](const size_t index) const;

  string operator +(const string &s) const;
  string operator +=(const string &s);

  bool operator == (const string &s) const;

  template <size_t T>
  bool operator == (const small_string<T> &s) const;

  bool operator != (const string &s) const;

  template <size_t T>
  bool operator != (const small_string<T> &s) const;

  utf8_string_iterator begin() const;
  utf8_string_iterator end() const;

  inline const char *c_str() const { return text; };

  bool StartsWith(const string &s) const;
  bool EndsWith(const string &s) const;

  bool Contains(const string &s) const;
};

template <size_t TCount>
lsResult string_Create(_Out_ string *pString, const char text[TCount]);

lsResult string_Create(_Out_ string *pString, _In_ const char *text);
lsResult string_Create(_Out_ string *pString, _In_ const char *text, const size_t size);

template <size_t TCount>
lsResult string_Create(_Out_ string *pString, const wchar_t text[TCount]);

lsResult string_Create(_Out_ string *pString, const wchar_t *text);
lsResult string_Create(_Out_ string *pString, const wchar_t *text, const size_t bufferSize);
lsResult string_Create(_Out_ string *pString, const string &from);

template <typename ...Args>
__declspec(deprecated) lsResult string_CreateFormat(_Out_ string *pString, _In_ const char *formatString, Args&&... args);

template <typename ...Args>
lsResult string_Format(_Out_ string *pString, Args&&... args);

lsResult string_Destroy(string *pString);

lsResult string_Reserve(string &str, const size_t size);

lsResult string_GetByteSize(const string &str, _Out_ size_t *pSize);
lsResult string_GetCount(const string &str, _Out_ size_t *pLength);

lsResult string_ToWideString(const string &str, _Out_ wchar_t *pWideString, const size_t bufferCount);
lsResult string_ToWideString(const string &str, _Out_ wchar_t *pWideString, const size_t bufferCount, _Out_ size_t *pWideStringCount);
lsResult string_GetRequiredWideStringCount(const string &str, _Out_ size_t *pWideStringCount);

lsResult string_ToWideStringRaw(const char *string, _Out_ wchar_t *pWideString, const size_t bufferCount, _Out_opt_ size_t *pWideStringCount = nullptr);

lsResult string_Substring(const string &text, _Out_ string *pSubstring, const size_t startCharacter);
lsResult string_Substring(const string &text, _Out_ string *pSubstring, const size_t startCharacter, const size_t length);

lsResult string_Append(string &text, const string &appendedText);
lsResult string_Append(string &text, const char *appendedText);
lsResult string_Append(string &text, const char *appendedText, const size_t size);

template <typename ...Args>
lsResult string_AppendFormat(string &text, Args&&... args);

lsResult string_AppendUnsignedInteger(string &text, const uint64_t value);
lsResult string_AppendInteger(string &text, const int64_t value);
lsResult string_AppendBool(string &text, const bool value);
lsResult string_AppendDouble(string &text, const double_t value);

lsResult string_ToDirectoryPath(_Out_ string *pString, const string &text);
lsResult string_ToFilePath(_Out_ string *pString, const string &text);

lsResult string_Equals(const string &strA, const string &strB, bool *pAreEqual);

lsResult string_StartsWith(const string &strA, const string &start, _Out_ bool *pStartsWith);
lsResult string_EndsWith(const string &strA, const string &end, _Out_ bool *pEndsWith);

lsResult string_FindFirst(const string &str, const string &find, _Out_ size_t *pStartChar, _Out_ bool *pContained);
lsResult string_Contains(const string &strA, const string &contained, _Out_ bool *pContains);

lsResult string_RemoveChar(const string &str, const utf32_t remove, _Out_ string *pResult);
lsResult string_RemoveString(const string &str, const string &remove, _Out_ string *pResult);

lsResult string_Replace(const string &str, const string &replace, const string &with, _Out_ string *pResult);

lsResult string_TrimStart(const string &str, const utf32_t trimmedChar, _Out_ string *pTrimmedString);
lsResult string_TrimEnd(const string &str, const utf32_t trimmedChar, _Out_ string *pTrimmedString);

// parameters to the lambda:
//   utf32_t: utf-8 codepoint for comparisons
//   const char *: start of first byte of the char
//   size_t: bytes of the utf-8 char.
lsResult string_ForEachChar(const string &str, const std::function<lsResult (utf32_t, const char *, size_t)> &function);

lsResult small_string_GetCount_Internal(_In_ const char *text, const size_t maxSize, _Out_ size_t *pCount, _Out_ size_t *pSize);
bool small_string_StringsAreEqual_Internal(_In_ const char *textA, _In_ const char *textB, const size_t bytes, const size_t count);

//////////////////////////////////////////////////////////////////////////

template<size_t TCount>
inline utf32_t lsToChar(const char c[TCount])
{
  return lsToChar(c, TCount);
}

template<size_t TCount>
inline lsResult string_Create(_Out_ string *pString, const char text[TCount])
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Create(pString, text, TCount));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult string_Create(_Out_ string *pString, const wchar_t text[TCount])
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Create(pString, text, TCount));

epilogue:
  return result;
}

template<typename ...Args>
inline lsResult string_Format(_Out_ string *pString, Args && ...args)
{
  lsResult result = lsR_Success;

  const size_t maxCapacity = sformat_capacity(args...);

  LS_ERROR_CHECK(string_Create(pString, "", 1));
  
  LS_ERROR_CHECK(string_Reserve(*pString, maxCapacity));
  LS_ERROR_IF(!sformat_to(pString->text, maxCapacity, args...), lsR_InternalError);

  size_t count, bytes;
  LS_ERROR_CHECK(small_string_GetCount_Internal(pString->text, maxCapacity, &count, &bytes));

  pString->count = count;
  pString->bytes = bytes;

epilogue:
  if (LS_FAILED(result))
    string_Destroy(pString);

  return result;
}

template <typename ...Args>
lsResult string_AppendFormat(string &text, Args && ... args)
{
  lsResult result = lsR_Success;

  const size_t maxAdditionalCapacity = sformat_capacity(args...);
  const size_t bytesWithoutNullTerminator = lsMin(text.bytes, text.bytes - 1);

  LS_ERROR_CHECK(string_Reserve(text, bytesWithoutNullTerminator + maxAdditionalCapacity));

  LS_ERROR_IF(!sformat_to(text.text + bytesWithoutNullTerminator, maxAdditionalCapacity, args...), lsR_InternalError);

  size_t count, bytes;
  LS_ERROR_CHECK(small_string_GetCount_Internal(text.text + bytesWithoutNullTerminator, maxAdditionalCapacity, &count, &bytes));

  text.count = lsMin(text.count, text.count - 1) + count;
  text.bytes = bytesWithoutNullTerminator + bytes;

epilogue:
  if (LS_FAILED(result))
    text.text[bytesWithoutNullTerminator] = '\0';

  return result;
}

template<size_t TSize>
inline string::string(const char text[TSize]) : string(text, TSize)
{ }

template<size_t TSize>
inline string::string(const wchar_t text[TSize]) : string(text, TSize)
{ }

//////////////////////////////////////////////////////////////////////////

template <size_t TCount>
struct small_string
{
  char text[TCount + 1] = "";
  size_t bytes = 0;
  size_t count = 0;

  small_string() = default;

  small_string(const small_string<TCount> &copy);
  small_string(small_string<TCount> &&move);

  small_string<TCount> &operator = (const small_string<TCount> &copy);
  small_string<TCount> &operator = (small_string<TCount> &&move);

  const char *c_str() const;

  template <size_t TOtherCount>
  bool operator == (const small_string<TOtherCount> &other) const;

  template <size_t TOtherCount>
  bool operator != (const small_string<TOtherCount> &other) const;

  bool operator == (const string &other) const;
  bool operator != (const string &other) const;

  utf8_string_iterator begin() const;
  utf8_string_iterator end() const;

  operator const string() const;
};

template <size_t TCount>
lsResult small_string_Create(_Out_ small_string<TCount> *pStackString);

template <size_t TCount, size_t TTextCount>
lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const char text[TTextCount]);

template <size_t TCount>
lsResult small_string_CreateRaw(_Out_ small_string<TCount> *pStackString, _In_ const char *text);

template <size_t TCount>
lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, _In_ const char *text, const size_t size);

template <size_t TCount, size_t TTextCount>
lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const small_string<TTextCount> &text);

template <size_t TCount>
lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const string &text);

template <size_t TCount>
lsResult string_Create(_Out_ string *pString, const small_string<TCount> &stackString);

template <size_t TCount>
lsResult string_Append(string &text, const small_string<TCount> &appendedText);

template <size_t TCount>
lsResult small_string_GetByteSize(const small_string<TCount> &str, _Out_ size_t *pSize);

template <size_t TCount>
lsResult small_string_GetCount(const small_string<TCount> &str, _Out_ size_t *pLength);

//////////////////////////////////////////////////////////////////////////

template<size_t TCount>
inline small_string<TCount>::small_string(const small_string<TCount> &copy) :
  bytes(copy.bytes),
  count(copy.count)
{
  mMemcpy(text, copy.text, bytes / sizeof(char));
}

template<size_t TCount>
inline small_string<TCount>::small_string(small_string<TCount> &&move) :
  bytes(move.bytes),
  count(move.count)
{
  mMemmove(text, move.text, bytes / sizeof(char));
}

template<size_t TCount>
inline small_string<TCount> &small_string<TCount>::operator=(const small_string<TCount> &copy)
{
  bytes = copy.bytes;
  count = copy.count;
  mMemcpy(text, copy.text, bytes / sizeof(char));

  return *this;
}

template<size_t TCount>
inline small_string<TCount> &small_string<TCount>::operator=(small_string<TCount> &&move)
{
  bytes = move.bytes;
  count = move.count;
  mMemmove(text, move.text, bytes / sizeof(char));

  return *this;
}

template<size_t TCount>
inline const char *small_string<TCount>::c_str() const
{
  return this->text;
}

template<size_t TCount>
inline utf8_string_iterator small_string<TCount>::begin() const
{
  return utf8_string_iterator((char *)text, bytes);
}

template<size_t TCount>
inline utf8_string_iterator small_string<TCount>::end() const
{
  return utf8_string_iterator(nullptr, 0);
}

template<size_t TCount>
template<size_t TOtherCount>
inline bool small_string<TCount>::operator==(const small_string<TOtherCount> &other) const
{
  if (other.bytes != bytes || other.count != this->count)
    return false;

  return small_string_StringsAreEqual_Internal(text, other.text, bytes, count);
}

template<size_t TCount>
template<size_t TOtherCount>
inline bool small_string<TCount>::operator!=(const small_string<TOtherCount> &other) const
{
  return !(*this == other);
}

template<size_t TCount>
bool small_string<TCount>::operator == (const string &other) const
{
  if (other.bytes != bytes || other.count != this->count || other.hasFailed)
    return false;

  return small_string_StringsAreEqual_Internal(text, other.text, bytes, count);
}

template<size_t TCount>
bool small_string<TCount>::operator != (const string &other) const
{
  return !(*this == other);
}

template<size_t TCount>
inline small_string<TCount>::operator const string() const
{
  string constString;

  constString.bytes = bytes;
  constString.count = count;
  constString.text = const_cast<char *>(text);
  constString.capacity = TCount;

  return constString;
}

template<size_t TCount>
inline lsResult small_string_Create(_Out_ small_string<TCount> *pStackString)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pStackString == nullptr, lsR_ArgumentNull);

  pStackString->bytes = 1;
  pStackString->count = 1;
  LS_ERROR_CHECK(mMemset(pStackString->text, TCount + 1, 0));

epilogue:
  return result;
}

template<size_t TCount, size_t TTextCount>
inline lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const char text[TTextCount])
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(small_string_Create(pStackString, (char *)text, TTextCount));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult small_string_CreateRaw(_Out_ small_string<TCount> *pStackString, _In_ const char *text)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(text == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(small_string_Create(pStackString, text, strlen(text) + 1));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, _In_ const char *text, const size_t size)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pStackString == nullptr || text == nullptr, lsR_ArgumentNull);

  size_t textSize;
  size_t textCount;

  LS_ERROR_CHECK(small_string_GetCount_Internal(text, size, &textCount, &textSize));

  const bool isNotNullTerminated = *(text + textSize - 1) != '\0';

  LS_ERROR_IF(textSize + isNotNullTerminated > TCount, lsR_ArgumentOutOfBounds);

  pStackString->bytes = textSize + isNotNullTerminated;
  pStackString->count = textCount + isNotNullTerminated;
  LS_ERROR_CHECK(mMemcpy(pStackString->text, text, textSize));

  if (isNotNullTerminated)
    pStackString->text[textSize] = '\0';

epilogue:
  return result;
}

template<size_t TCount, size_t TTextCount>
inline lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const small_string<TTextCount> &text)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pStackString == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(TCount < text.bytes, lsR_ArgumentOutOfBounds);

  pStackString->bytes = text.bytes;
  pStackString->count = text.count;
  LS_ERROR_CHECK(mMemcpy(pStackString->text, text.text, pStackString->bytes));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult small_string_Create(_Out_ small_string<TCount> *pStackString, const string &text)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pStackString == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(TCount < text.bytes, lsR_ArgumentOutOfBounds);

  pStackString->bytes = text.bytes;
  pStackString->count = text.count;
  LS_ERROR_CHECK(mMemcpy(pStackString->text, text.text, pStackString->bytes));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult string_Create(_Out_ string *pString, const small_string<TCount> &stackString)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Create(pString, stackString.text, stackString.bytes));

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult string_Append(string &text, const small_string<TCount> &appendedText)
{
  lsResult result = lsR_Success;

  string s;
  s.capacity = s.bytes = appendedText.bytes;
  s.count = appendedText.count;
  s.hasFailed = false;
  s.text = const_cast<char *>(appendedText.text);

  LS_ERROR_CHECK(string_Append(text, s));

epilogue:
  // Prevent from attempting to free `s.text`.
  s.capacity = s.count = s.bytes = 0;
  s.text = nullptr;

  return result;
}

template<size_t TCount>
inline lsResult small_string_GetByteSize(const small_string<TCount> &str, _Out_ size_t *pSize)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pSize == nullptr, lsR_ArgumentNull);

  *pSize = str.bytes;

epilogue:
  return result;
}

template<size_t TCount>
inline lsResult small_string_GetCount(const small_string<TCount> &str, _Out_ size_t *pLength)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pLength == nullptr, lsR_ArgumentNull);

  *pLength = str.count;

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

template<size_t T>
inline bool string::operator==(const small_string<T> &s) const
{
  if (s.bytes <= 1 && bytes <= 1)
    return true;

  if (s.bytes != bytes || s.count != count)
    return false;

  string tmp;
  tmp.text = const_cast<char *>(s.text);
  tmp.capacity = tmp.bytes = s.bytes;
  tmp.count = s.count;

  const bool equal = (*this) == tmp;

  tmp.text = nullptr;

  return equal;
}

template<size_t T>
inline bool string::operator!=(const small_string<T> &s) const
{
  return !((*this) == s);
}

//////////////////////////////////////////////////////////////////////////

inline size_t sformat_GetMaxBytes(const string &str, const sformatState &)
{
  return str.bytes;
}

inline size_t _sformat_Append(const string &str, const sformatState &fs, char *text)
{
  return _sformat_AppendInplaceString(str.text, str.count, str.bytes, fs, text);
}

template <size_t TLength>
inline size_t sformat_GetMaxBytes(const small_string<TLength> &str, const sformatState &)
{
  return str.bytes;
}

template <size_t TLength>
inline size_t _sformat_Append(const small_string<TLength> &str, const sformatState &fs, char *text)
{
  return _sformat_AppendInplaceString(str.text, str.count, str.bytes, fs, text);
}
