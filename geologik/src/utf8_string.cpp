#include "utf8_string.h"

#include <string.h>

utf32_t lsToChar(IN const char *c, const size_t size)
{
  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, c, size);
  LS_DEBUG_ASSERT_TRUE(utf8::utf8_next(&iter) == 1);

  return utf32_t(iter.codepoint);
}

bool string_IsValidChar(const char *c, const size_t size, _Out_ OPTIONAL utf32_t *pChar /* = nullptr */, _Out_ OPTIONAL size_t *pCharSize /* = nullptr */)
{
  if (c == nullptr || size == 0)
    return false;

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, c, size);

  if (utf8::utf8_next(&iter) != 1)
    return false;

  if (pChar != nullptr)
    *pChar = (utf32_t)iter.codepoint;

  if (pCharSize != nullptr)
    *pCharSize = (size_t)iter.size;

  return true;
}

//////////////////////////////////////////////////////////////////////////

string::string() :
  text(nullptr),
  bytes(0),
  count(0),
  capacity(0),
  hasFailed(false)
{ }

string::string(IN const char *text, const size_t size) :
  text(nullptr),
  bytes(0),
  count(0),
  capacity(0),
  hasFailed(false)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Create(this, text, size));

epilogue:
  if (LS_FAILED(result))
  {
    this->~string();
    hasFailed = true;
  }
}

string::string(IN const char *text) : string(text, lsStringLength(text) + 1)
{ }

string::string(const wchar_t *text, const size_t size /* = nullptr */) : string()
{
  if (LS_FAILED(string_Create(this, text, size)))
  {
    this->~string();
    hasFailed = true;
  }
}

string::string(const wchar_t *text /* = nullptr */) : string(text, wcslen(text) + 1)
{ }

string::~string()
{
  if (text != nullptr)
    lsFreePtr(&text);

  text = nullptr;
  bytes = 0;
  count = 0;
  capacity = 0;
  hasFailed = false;
}

string::string(const string &copy) :
  text(nullptr),
  bytes(0),
  count(0),
  capacity(0),
  hasFailed(false)
{
  lsResult result = lsR_Success;

  if (copy.hasFailed)
    goto epilogue;

  if (copy.capacity == 0)
    goto epilogue;

  LS_ERROR_CHECK(lsAllocZero(&text, copy.bytes));
  capacity = bytes = copy.bytes;
  count = copy.count;

  lsMemcpy(text, copy.text, bytes);

epilogue:
  if (LS_FAILED(result))
  {
    this->~string();
    hasFailed = true;
  }
}

string::string(string &&move) :
  text(move.text),
  bytes(move.bytes),
  count(move.count),
  capacity(move.capacity),
  hasFailed(move.hasFailed)
{
  move.text = nullptr;
  move.count = 0;
  move.bytes = 0;
  move.capacity = 0;
  move.hasFailed = false;
}

string &string::operator=(const string &copy)
{
  lsResult result = lsR_Success;

  this->hasFailed = false;

  if (this->capacity < copy.bytes)
  {
    LS_ERROR_CHECK(lsRealloc(&text, copy.bytes));
    this->capacity = copy.bytes;
    lsMemcpy(text, copy.text, copy.bytes);
  }
  else if (copy.bytes > 0)
  {
    lsMemcpy(text, copy.text, copy.bytes);
  }

  this->bytes = copy.bytes;
  this->count = copy.count;

epilogue:
  if (LS_FAILED(result))
    this->hasFailed = true;

  return *this;
}

string &string::operator=(string &&move)
{
  if (move.text == nullptr)
  {
    this->bytes = 0;
    this->count = 0;
    this->hasFailed = move.hasFailed;

    if (this->text != nullptr)
      this->text[0] = '\0';
  }
  else
  {
    this->~string();
    new (this) string(std::move(move));
  }

  return *this;
}

size_t string::Size() const
{
  return bytes;
}

size_t string::Count() const
{
  return count;
}

utf32_t string::operator[](const size_t index) const
{
  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, text, bytes);

  if (index >= count)
  {
    lsFail(); // out of bounds!
    return (utf32_t)iter.codepoint;
  }

  if (index == count - 1)
    return (utf32_t)0;

  for (size_t i = 0; i <= index; i++)
    LS_DEBUG_ASSERT_TRUE(utf8::utf8_next(&iter) == 1);

  return (utf32_t)iter.codepoint;
}

string string::operator+(const string &s) const
{
  if (s.bytes <= 1 || s.count <= 1 || s.c_str() == nullptr)
    return *this;
  else if (this->bytes <= 1 || this->count <= 1 || this->c_str() == nullptr)
    return s;

  string ret;
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(lsAllocZero(&ret.text, this->bytes + s.bytes - 1));
  ret.capacity = this->bytes + s.bytes - 1;
  lsMemcpy(ret.text, this->text, this->bytes - 1);
  lsMemcpy(ret.text + this->bytes - 1, s.text, s.bytes);
  ret.bytes = ret.capacity;
  ret.count = s.count + count - 1;

epilogue:
  if (LS_FAILED(result))
  {
    ret = string();
    ret.hasFailed = true;
  }

  return ret;
}

string string::operator+=(const string &s)
{
  if (hasFailed || s.hasFailed)
    return *this;

  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Append(*this, s));

epilogue:
  if (LS_FAILED(result))
    hasFailed = true;
  
  return *this;
}

bool string::operator==(const string &s) const
{
  bool ret;

  string_Equals(*this, s, &ret);

  return ret;
}

bool string::operator!=(const string &s) const
{
  return !(*this == s);
}

utf8_string_iterator string::begin() const
{
  return utf8_string_iterator(text, bytes);
}

utf8_string_iterator string::end() const
{
  return utf8_string_iterator(nullptr, 0);
}

bool string::StartsWith(const string &s) const
{
  bool startsWith = false;

  const lsResult result = string_StartsWith(*this, s, &startsWith);

  return LS_SUCCESS(result) && startsWith;
}

bool string::EndsWith(const string &s) const
{
  bool endsWith = false;

  const lsResult result = string_EndsWith(*this, s, &endsWith);

  return LS_SUCCESS(result) && endsWith;
}

bool string::Contains(const string &s) const
{
  bool contained = false;

  const lsResult result = string_Contains(*this, s, &contained);

  return LS_SUCCESS(result) && contained;
}

lsResult string_Create(_Out_ string *pString, IN const char *text /* = nullptr */)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text == nullptr)
  {
    pString->hasFailed = false;

    if (pString->text != nullptr)
    {
      pString->text[0] = '\0';
      pString->count = 1;
      pString->bytes = 1;
    }

    goto epilogue; // success!
  }

  LS_ERROR_CHECK(string_Create(pString, text, lsStringLength(text) + 1));

epilogue:
  return result;
}

lsResult string_Create(_Out_ string *pString, IN const char *text, size_t size)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text == nullptr)
  {
    pString->hasFailed = false;

    if (pString->text != nullptr)
    {
      pString->text[0] = '\0';
      pString->count = 1;
      pString->bytes = 1;
    }

    goto epilogue; // success?
  }

  size = lsStringLength(text, size);
  size++;

  if (pString->capacity < size)
  {
    LS_ERROR_CHECK(lsRealloc(&pString->text, size));
    pString->capacity = pString->bytes = size;
  }

  pString->text[0] = '\0';
  pString->bytes = size;

  lsMemcpy(pString->text, text, pString->bytes - 1);
  pString->text[pString->bytes - 1] = '\0';

  pString->count = 0;

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, text, size - 1);

  while (true)
  {
    if (1 != utf8::utf8_next(&iter))
      break;

    pString->count++;
  }

  pString->count++;

epilogue:
  return result;
}

lsResult string_Create(_Out_ string *pString, const wchar_t *text /* = nullptr */)
{
  lsResult result = lsR_Success;

  BOOL bFalse = FALSE;
  size_t size;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text == nullptr)
  {
    pString->hasFailed = false;

    if (pString->text != nullptr)
    {
      pString->text[0] = '\0';
      pString->count = 1;
      pString->bytes = 1;
    }

    goto epilogue; // success?
  }

  size = wcslen(text) + 1;

  *pString = string();
  
  if (pString->capacity < size * string_MaxUtf16CharInUtf8Chars)
  {
    LS_ERROR_CHECK(lsRealloc(&pString->text, size * string_MaxUtf16CharInUtf8Chars));
    pString->capacity = size * string_MaxUtf16CharInUtf8Chars;
  }

  if (0 == (pString->bytes = WideCharToMultiByte(CP_UTF8, 0, text, (int32_t)size, pString->text, (int32_t)pString->capacity, nullptr, &bFalse)))
  {
    DWORD error = GetLastError();
    (void)error;

    LS_ERROR_SET(lsR_InvalidParameter);
  }

  pString->count = 0;

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, pString->text, pString->bytes);

  while (true)
  {
    if (1 != utf8::utf8_next(&iter))
      break;

    pString->count++;
  }

  pString->count++;

epilogue:
  return result;
}

lsResult string_Create(_Out_ string *pString, const wchar_t *text, const size_t bufferSize /* = nullptr */)
{
  lsResult result = lsR_Success;

  BOOL bFalse = FALSE;
  size_t size;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text == nullptr)
  {
    pString->hasFailed = false;

    if (pString->text != nullptr)
    {
      pString->text[0] = '\0';
      pString->count = 1;
      pString->bytes = 1;
    }

    goto epilogue; // success?
  }

  size = wcsnlen_s(text, bufferSize) + 1;

  *pString = string();

  if (pString->capacity < size * string_MaxUtf16CharInUtf8Chars)
  {
    LS_ERROR_CHECK(lsRealloc(&pString->text, size * string_MaxUtf16CharInUtf8Chars));
    pString->capacity = size * string_MaxUtf16CharInUtf8Chars;
  }

  if (0 == (pString->bytes = WideCharToMultiByte(CP_UTF8, 0, text, (int32_t)(size - 1), pString->text, (int32_t)pString->capacity, nullptr, &bFalse)))
  {
    DWORD error = GetLastError();
    (void)error;

    LS_ERROR_SET(lsR_InvalidParameter);
  }

  pString->text[pString->bytes] = '\0';
  pString->bytes++;

  pString->count = 0;

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, pString->text, pString->bytes);

  while (true)
  {
    if (1 != utf8::utf8_next(&iter))
      break;

    pString->count++;
  }

  pString->count++;

epilogue:
  return result;
}

lsResult string_Create(_Out_ string *pString, const string &from /* = nullptr */)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (from.bytes <= 1)
  {
    if (pString->bytes > 1)
    {
      pString->bytes = 1;
      pString->count = 1;
      pString->text[0] = '\0';
    }

    goto epilogue; // success?
  }

  *pString = string();

  if (pString->capacity < from.bytes)
  {
    LS_ERROR_CHECK(lsRealloc(&pString->text, from.bytes));
    pString->capacity = pString->bytes = from.bytes;
  }
  else
  {
    pString->bytes = from.bytes;
  }

  lsMemcpy(pString->text, from.text, pString->bytes);

  pString->count = from.count;

epilogue:
  return result;
}

lsResult string_Destroy(string *pString)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (pString->text != nullptr)
    lsFreePtr(&pString->text);

  pString->capacity = 0;
  pString->bytes = 0;
  pString->count = 0;
  pString->hasFailed = false;

epilogue:
  return result;
}

lsResult string_Reserve(string &string, const size_t size)
{
  lsResult result = lsR_Success;

  if (string.capacity < size)
  {
    LS_ERROR_CHECK(lsRealloc(&string.text, size));
    string.capacity = size;
  }

epilogue:
  return result;
}

lsResult string_GetByteSize(const string &string, _Out_ size_t *pSize)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pSize == nullptr, lsR_ArgumentNull);

  *pSize = string.bytes;

epilogue:
  return result;
}

lsResult string_GetCount(const string &string, _Out_ size_t *pLength)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pLength == nullptr, lsR_ArgumentNull);

  *pLength = string.count;

epilogue:
  return result;
}

lsResult string_ToWideString(const string &string, _Out_ wchar_t *pWideString, const size_t bufferCount)
{
  size_t _unused;

  return string_ToWideString(string, pWideString, bufferCount, &_unused);
}

lsResult string_ToWideString(const string &string, _Out_ wchar_t *pWideString, const size_t bufferCount, _Out_ size_t *pWideStringCount)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pWideString == nullptr || pWideStringCount == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(string.hasFailed, lsR_ResourceInvalid);

  if (string.text == nullptr)
  {
    LS_ERROR_IF(bufferCount == 0, lsR_ArgumentOutOfBounds);

    pWideString[0] = L'\0';
    *pWideStringCount = 1;
  }
  else
  {
    int32_t length = 0;

    if (0 >= (length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.text, (int32_t)string.bytes, pWideString, (int32_t)bufferCount)))
    {
      const DWORD error = GetLastError();

      switch (error)
      {
      case ERROR_INSUFFICIENT_BUFFER:
        LS_ERROR_SET(lsR_ArgumentOutOfBounds);

      case ERROR_NO_UNICODE_TRANSLATION:
        LS_ERROR_SET(lsR_InvalidParameter);

      case ERROR_INVALID_FLAGS:
      case ERROR_INVALID_PARAMETER:
      default:
        LS_ERROR_SET(lsR_InternalError);
      }
    }

    *pWideStringCount = length;
  }

epilogue:
  return result;
}

lsResult string_ToWideStringRaw(const char *string, _Out_ wchar_t *pWideString, const size_t bufferCount, OPTIONAL _Out_ size_t *pWideStringCount /* = nullptr */)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pWideString == nullptr, lsR_ArgumentNull);

  if (string == nullptr)
  {
    LS_ERROR_IF(bufferCount == 0, lsR_ArgumentOutOfBounds);

    pWideString[0] = L'\0';

    if (pWideStringCount != nullptr)
      *pWideStringCount = 1;
  }
  else
  {
    const size_t stringLength = (lsStringLength(string) + 1);
    LS_ERROR_IF(stringLength > INT32_MAX, lsR_ArgumentOutOfBounds);

    int32_t length = 0;

    if (0 >= (length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string, (int32_t)stringLength, pWideString, (int32_t)bufferCount)))
    {
      const DWORD error = GetLastError();

      switch (error)
      {
      case ERROR_INSUFFICIENT_BUFFER:
        LS_ERROR_SET(lsR_ArgumentOutOfBounds);

      case ERROR_NO_UNICODE_TRANSLATION:
        LS_ERROR_SET(lsR_InvalidParameter);

      case ERROR_INVALID_FLAGS:
      case ERROR_INVALID_PARAMETER:
      default:
        LS_ERROR_SET(lsR_InternalError);
      }
    }

    if (pWideStringCount != nullptr)
      *pWideStringCount = length;
  }

epilogue:
  return result;
}

lsResult string_GetRequiredWideStringCount(const string &string, _Out_ size_t *pWideStringCount)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pWideStringCount == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(string.hasFailed, lsR_ResourceInvalid);

  if (string.text == nullptr)
  {
    *pWideStringCount = 1;
  }
  else
  {
    int32_t length = 0;

    if (0 >= (length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.text, (int32_t)string.bytes, nullptr, 0)))
    {
      const DWORD error = GetLastError();

      switch (error)
      {
      case ERROR_INSUFFICIENT_BUFFER:
        LS_ERROR_SET(lsR_ArgumentOutOfBounds);

      case ERROR_NO_UNICODE_TRANSLATION:
        LS_ERROR_SET(lsR_InvalidParameter);

      case ERROR_INVALID_FLAGS:
      case ERROR_INVALID_PARAMETER:
      default:
        LS_ERROR_SET(lsR_InternalError);
      }
    }

    *pWideStringCount = length;
  }

epilogue:
  return result;
}

lsResult string_Substring(const string &text, _Out_ string *pSubstring, const size_t startCharacter)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pSubstring == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(startCharacter >= text.count, lsR_ArgumentOutOfBounds);

  LS_ERROR_CHECK(string_Substring(text, pSubstring, startCharacter, text.count - startCharacter - 1));

epilogue:
  return result;
}

lsResult string_Substring(const string &text, _Out_ string *pSubstring, const size_t startCharacter, const size_t length)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pSubstring == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(startCharacter >= text.count || startCharacter + length >= text.count, lsR_ArgumentOutOfBounds);

  LS_ERROR_CHECK(string_Reserve(*pSubstring, length * 4));

  {
    utf8::utf8_iter iter;
    utf8::utf8_initEx(&iter, text.text, text.bytes);
    size_t character = 0;

    while (character < startCharacter)
    {
      LS_ERROR_IF(utf8::utf8_next(&iter) != 1, lsR_InternalError);
      character++;
    }

    {
      *pSubstring = string();
      
      while (character - startCharacter < length)
      {
        LS_ERROR_IF(utf8::utf8_next(&iter) != 1, lsR_InternalError);
        const size_t characterSize = iter.size;
        character++;

        LS_ERROR_CHECK(string_Append(*pSubstring, text.text + iter.position, characterSize));
      }
    }
  }

epilogue:
  return result;
}

lsResult string_Append(string &text, const string &appendedText)
{
  lsResult result = lsR_Success;

  if (text.bytes > 0 && appendedText.bytes > 0)
  {
    if (text.capacity < text.bytes + appendedText.bytes - 1)
    {
      size_t newCapacity;

      if (text.capacity * 2 >= text.bytes + appendedText.bytes - 1)
        newCapacity = text.capacity * 2;
      else
        newCapacity = text.bytes + appendedText.bytes - 1;

      LS_ERROR_CHECK(lsRealloc(&text.text, newCapacity));
      text.capacity = newCapacity;
    }

    lsMemcpy(text.text + text.bytes - 1, appendedText.text, appendedText.bytes);

    text.count += appendedText.count - 1;
    text.bytes += appendedText.bytes - 1;
  }
  else
  {
    if (appendedText.bytes == 0)
    {
      goto epilogue; // success!
    }
    else
    {
      text.hasFailed = false;

      if (text.capacity < appendedText.bytes)
      {
        LS_ERROR_CHECK(lsRealloc(&text.text, appendedText.bytes));
        text.capacity = appendedText.bytes;
        lsMemcpy(text.text, appendedText.text, appendedText.bytes);
      }
      else if (appendedText.bytes > 0)
      {
        lsMemcpy(text.text, appendedText.text, appendedText.bytes);
      }

      text.bytes = appendedText.bytes;
      text.count = appendedText.count;

      goto epilogue; // success!
    }
  }

epilogue:
  return result;
}

lsResult string_Append(string &text, const char *appendedText)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(appendedText == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(string_Append(text, appendedText, lsStringLength(appendedText)));

epilogue:
  return result;
}

lsResult string_Append(string &text, const char *appendedText, const size_t size)
{
  lsResult result = lsR_Success;
  size_t count, bytes;

  LS_ERROR_IF(appendedText == nullptr, lsR_ArgumentNull);
  LS_ERROR_CHECK(small_string_GetCount_Internal(appendedText, size, &count, &bytes));

  {
    const bool appendedTextNotTerminated = *(appendedText + bytes - 1) != '\0';

    if (text.bytes > 0 && bytes > 0)
    {
      if (text.capacity < text.bytes + bytes - 1 + appendedTextNotTerminated)
      {
        size_t newCapacity;

        if (text.capacity * 2 >= text.bytes + bytes - 1 + appendedTextNotTerminated)
          newCapacity = text.capacity * 2;
        else
          newCapacity = text.bytes + bytes - 1 + appendedTextNotTerminated;

        LS_ERROR_CHECK(lsRealloc(&text.text, newCapacity));
        text.capacity = newCapacity;
      }

      lsMemcpy(text.text + text.bytes - 1, appendedText, bytes);

      if (!appendedTextNotTerminated)
      {
        text.count += count - 1;
        text.bytes += bytes - 1;
      }
      else
      {
        text.text[text.bytes - 1 + bytes] = '\0';

        text.count += count;
        text.bytes += bytes;
      }
    }
    else
    {
      if (bytes == 0)
      {
        goto epilogue; // success!
      }
      else
      {
        text.hasFailed = false;

        if (text.capacity < bytes + appendedTextNotTerminated)
        {
          LS_ERROR_CHECK(lsRealloc(&text.text, bytes + appendedTextNotTerminated));
          text.capacity = bytes + appendedTextNotTerminated;

          lsMemcpy(text.text, appendedText, bytes);
        }
        else if (bytes > 0)
        {
          lsMemcpy(text.text, appendedText, bytes);
        }

        if (!appendedTextNotTerminated)
        {
          text.bytes = bytes;
          text.count = count;
        }
        else
        {
          text.text[bytes] = '\0';

          text.bytes = bytes + 1;
          text.count = count + 1;
        }

        goto epilogue; // success!
      }
    }
  }

epilogue:
  return result;
}

lsResult string_AppendUnsignedInteger(string &text, const uint64_t value)
{
  lsResult result = lsR_Success;

  char txt[64];
  _ui64toa(value, txt, 10);

  LS_ERROR_CHECK(string_Append(text, txt, sizeof(txt)));

epilogue:
  return result;
}

lsResult string_AppendInteger(string &text, const int64_t value)
{
  lsResult result = lsR_Success;

  char txt[64];
  _i64toa(value, txt, 10);

  LS_ERROR_CHECK(string_Append(text, txt, sizeof(txt)));

epilogue:
  return result;
}

lsResult string_AppendBool(string &text, const bool value)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(string_Append(text, value ? "true" : "false"));

epilogue:
  return result;
}

lsResult string_AppendDouble(string &text, const double_t value)
{
  lsResult result = lsR_Success;

  char txt[128];
  LS_ERROR_IF(0 > sprintf_s(txt, "%f", value), lsR_InternalError);

  LS_ERROR_CHECK(string_Append(text, txt, sizeof(txt)));

epilogue:
  return result;
}

lsResult string_ToDirectoryPath(_Out_ string *pString, const string &text)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text.text == nullptr || text.bytes <= 1 || text.count <= 1)
  {
    *pString = "";
    goto epilogue; // success!
  }

  LS_ERROR_CHECK(string_Reserve(*pString, text.bytes + 1));
  *pString = text;
  LS_ERROR_IF(pString->hasFailed, lsR_InternalError);

  {
    size_t offset = 0;
    bool lastWasSlash = false;

    for (size_t i = 0; i < pString->count && pString->bytes - offset > 1; ++i)
    {
      utf8::utf8_iter iter;
      utf8::utf8_initEx(&iter, pString->text + offset, pString->bytes - offset);
      LS_ERROR_IF(1 != utf8::utf8_next(&iter), lsR_InternalError);
      const size_t characterSize = iter.size;

      if (characterSize == 1)
      {
        if (lastWasSlash && (pString->text[offset] == '/' || pString->text[offset] == '\\') && i != 1)
        {
          pString->bytes -= characterSize;
          lsMemmove(&pString->text[offset], &pString->text[offset + characterSize], pString->bytes - offset);
          --pString->count;
          --i;
          continue;
        }
        else if (pString->text[offset] == '/')
        {
          pString->text[offset] = '\\';
          lastWasSlash = true;
        }
        else if (pString->text[offset] == '\\')
        {
          lastWasSlash = true;
        }
        else
        {
          lastWasSlash = false;
        }
      }
      else
      {
        lastWasSlash = false;
      }

      offset += (size_t)characterSize;
    }

    if (!lastWasSlash)
    {
      pString->text[offset] = '\\';
      pString->text[offset + 1] = '\0';
      ++pString->bytes;
      ++pString->count;
    }
  }

epilogue:
  return result;
}

lsResult string_ToFilePath(_Out_ string *pString, const string &text)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pString == nullptr, lsR_ArgumentNull);

  if (text.text == nullptr || text.bytes <= 1 || text.count <= 1)
  {
    *pString = "";
    goto epilogue; // success!
  }

  *pString = text;
  LS_ERROR_IF(pString->hasFailed, lsR_InternalError);

  {
    size_t offset = 0;
    bool lastWasSlash = false;

    for (size_t i = 0; i < pString->count && pString->bytes - offset > 1; ++i)
    {
      utf8::utf8_iter iter;
      utf8::utf8_initEx(&iter, text.text + offset, text.bytes - offset);
      LS_ERROR_IF(1 != utf8::utf8_next(&iter), lsR_InternalError);
      const size_t characterSize = iter.size;

      if (characterSize == 1)
      {
        if (lastWasSlash && (pString->text[offset] == '/' || pString->text[offset] == '\\') && i != 1)
        {
          pString->bytes -= characterSize;
          lsMemmove(&pString->text[offset], &pString->text[offset + characterSize], pString->bytes - offset);
          --pString->count;
          continue;
        }
        else if (pString->text[offset] == '/')
        {
          pString->text[offset] = '\\';
          lastWasSlash = true;
        }
        else if (pString->text[offset] == '\\')
        {
          lastWasSlash = true;
        }
        else
        {
          lastWasSlash = false;
        }
      }
      else
      {
        lastWasSlash = false;
      }

      offset += (size_t)characterSize;

      if (characterSize == 0)
        break;
    }
  }

epilogue:
  return result;
}

lsResult string_Equals(const string &stringA, const string &stringB, bool *pAreEqual)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pAreEqual == nullptr, lsR_ArgumentNull);

  if ((stringA.bytes <= 1) && (stringB.bytes <= 1)) // "" should equal an uninitialized string.
  {
    *pAreEqual = true;
    goto epilogue; // success!
  }

  if (stringA.bytes != stringB.bytes || stringA.count != stringB.count)
  {
    *pAreEqual = false;
    goto epilogue; // success!
  }

  utf8::utf8_iter iterA, iterB;
  utf8::utf8_initEx(&iterA, stringA.text, stringA.bytes);
  utf8::utf8_initEx(&iterB, stringB.text, stringB.bytes);

  lsAssert(stringA.count);

  for (size_t i = 0; i < stringA.count - 1; ++i)
  {
    LS_ERROR_IF(1 != utf8::utf8_next(&iterA), lsR_InternalError);
    LS_ERROR_IF(1 != utf8::utf8_next(&iterB), lsR_InternalError);

    if (iterA.codepoint != iterB.codepoint)
    {
      *pAreEqual = false;
      goto epilogue; // success!
    }
  }

  *pAreEqual = true;

epilogue:
  return result;
}

lsResult string_ForEachChar(const string &str, const std::function<lsResult(utf32_t, const char *, size_t)> &function)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(str.hasFailed, lsR_InvalidParameter);
  LS_ERROR_IF(function == nullptr, lsR_ArgumentNull);

  for (const auto &_c : str)
    LS_ERROR_CHECK(function((utf32_t)_c.codePoint, _c.character, _c.characterSize));

epilogue:
  return result;
}

lsResult string_StartsWith(const string &stringA, const string &start, _Out_ bool *pStartsWith)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pStartsWith == nullptr, lsR_ArgumentNull);

  *pStartsWith = false;

  LS_ERROR_IF(stringA.hasFailed || start.hasFailed, lsR_InvalidParameter);

  if (stringA.bytes <= 1 || start.bytes <= 1)
  {
    *pStartsWith = start.bytes <= 1;
    goto epilogue; // success!
  }

  if (start.count > stringA.count)
    goto epilogue; // pStartsWith is already false.

  utf8::utf8_iter iterA, iterB;
  utf8::utf8_initEx(&iterA, stringA.text, stringA.bytes);
  utf8::utf8_initEx(&iterB, start.text, start.bytes);

  *pStartsWith = true;

  lsAssert(start.count);

  for (size_t i = 0; i < start.count - 1; ++i) // Exclude null char.
  {
    LS_ERROR_IF(1 != utf8::utf8_next(&iterA), lsR_InternalError);
    LS_ERROR_IF(1 != utf8::utf8_next(&iterB), lsR_InternalError);

    if (iterA.codepoint != iterB.codepoint)
    {
      *pStartsWith = false;
      break;
    }
  }

epilogue:
  return result;
}

lsResult string_EndsWith(const string &stringA, const string &end, _Out_ bool *pEndsWith)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pEndsWith == nullptr, lsR_ArgumentNull);

  *pEndsWith = false;

  LS_ERROR_IF(stringA.hasFailed || end.hasFailed, lsR_InvalidParameter);

  if (stringA.bytes <= 1 || end.bytes <= 1)
  {
    *pEndsWith = end.bytes <= 1;
    goto epilogue; // success!
  }

  if (end.count > stringA.count)
    goto epilogue;

  utf8::utf8_iter iterA, iterB;
  utf8::utf8_initEx(&iterA, stringA.text, stringA.bytes);
  utf8::utf8_initEx(&iterB, end.text, end.bytes);

  for (size_t i = 0; i < stringA.count - end.count; ++i) // Exclude null char.
  {
    LS_ERROR_IF(1 != utf8::utf8_next(&iterA), lsR_InternalError);
  }

  *pEndsWith = true;

  for (size_t i = 0; i < end.count - 1; ++i) // Exclude null char.
  {
    LS_ERROR_IF(1 != utf8::utf8_next(&iterA), lsR_InternalError);
    LS_ERROR_IF(1 != utf8::utf8_next(&iterB), lsR_InternalError);

    if (iterA.codepoint != iterB.codepoint)
    {
      *pEndsWith = false;
      break;
    }
  }

epilogue:
  return result;
}

// `pKmp` should be zeroed.
void string_GetKmp(const string &string, utf32_t *pString, size_t *pKmp)
{
  pKmp[0] = 0;

  const size_t length = string.count - 1;
  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, string.text, string.bytes);

  for (size_t i = 0; i < length; i++)
  {
    LS_DEBUG_ASSERT_TRUE(1 == utf8::utf8_next(&iter));
    pString[i] = (utf32_t)iter.codepoint;
  }

  for (size_t i = 1, k = 0; i < length; ++i)
  {
    while (k > 0 && pString[k] != pString[i])
      k = pKmp[k - 1];

    if (pString[k] == pString[i])
      ++k;

    pKmp[i] = k;
  }
}

void string_FindNextWithKMP(const string &string, const size_t offsetChar, const size_t offsetBytes, IN const utf32_t *pFind, const size_t findCountWithoutNull, IN const size_t *pKmp, _Out_ size_t *pStartChar, _Out_ size_t *pOffset, _Out_ bool *pContained)
{
  *pContained = false;

  const size_t chars = string.count - offsetChar - 1;

  if (findCountWithoutNull == 0 || findCountWithoutNull > chars || string.bytes <= offsetBytes)
    return;

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, string.text + offsetBytes, string.bytes - offsetBytes);

  for (size_t i = 0, k = 0; i < chars; i++)
  {
    LS_DEBUG_ASSERT_TRUE(1 == utf8::utf8_next(&iter));

    while (k > 0 && pFind[k] != iter.codepoint)
      k = pKmp[k - 1];

    if (pFind[k] == iter.codepoint)
      ++k;

    if (k == findCountWithoutNull)
    {
      *pStartChar = offsetChar + i - k + 1 + findCountWithoutNull;
      *pOffset = offsetBytes + iter.position + iter.size;
      *pContained = true;

      return;
    }
  }
}

lsResult string_FindFirst(const string &str, const string &find, _Out_ size_t *pStartChar, _Out_ bool *pContained)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  LS_ERROR_IF(pContained == nullptr || pStartChar == nullptr, lsR_ArgumentNull);

  *pContained = false;

  LS_ERROR_IF(str.hasFailed || find.hasFailed, lsR_InvalidParameter);
  LS_ERROR_IF(pStartChar == nullptr || pContained == nullptr, lsR_InvalidParameter);

  if (str.count <= 1 || find.count <= 1)
  {
    *pStartChar = 0;
    *pContained = find.count <= 1;
    goto epilogue; // success?
  }

  // TODO: Add a brute force variant for small strings.

  LS_ERROR_CHECK(lsAllocZero(&pKmp, find.count - 1));
  LS_ERROR_CHECK(lsAlloc(&pChars, find.count - 1));

  string_GetKmp(find, pChars, pKmp);

  size_t _unusedOffset;
  string_FindNextWithKMP(str, 0, 0, pChars, find.count - 1, pKmp, pStartChar, &_unusedOffset, pContained);

  if (*pContained)
    *pStartChar -= (find.count - 1);

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

lsResult string_Contains(const string &str, const string &contained, _Out_ bool *pContains)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  LS_ERROR_IF(pContains == nullptr, lsR_ArgumentNull);

  *pContains = false;

  LS_ERROR_IF(str.hasFailed || contained.hasFailed, lsR_InvalidParameter);

  if (str.count <= 1 || contained.count <= 1)
  {
    *pContains = contained.count <= 1;
    goto epilogue; // success!
  }

  // TODO: Add a brute force variant for small strings.

  LS_ERROR_CHECK(lsAllocZero(&pKmp, contained.count - 1));
  LS_ERROR_CHECK(lsAlloc(&pChars, contained.count - 1));

  string_GetKmp(contained, pChars, pKmp);

  size_t _unusedStartChar, _unusedOffset;
  string_FindNextWithKMP(str, 0, 0, pChars, contained.count - 1, pKmp, &_unusedStartChar, &_unusedOffset, pContains);

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

lsResult string_TrimStart(const string &str, const utf32_t trimmedChar, _Out_ string *pTrimmedString)
{
  lsResult result = lsR_Success;
  size_t charOffset = 0;

  LS_ERROR_IF(pTrimmedString == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(str.hasFailed || trimmedChar == 0, lsR_InvalidParameter);

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, str.text, str.bytes);

  for (; charOffset < str.count - 1; charOffset++)
  {
    LS_ERROR_IF(1 != utf8::utf8_next(&iter), lsR_InternalError);

    if (iter.codepoint != trimmedChar)
      break;
  }

  LS_ERROR_CHECK(string_Create(pTrimmedString, ""));
  LS_ERROR_CHECK(string_Reserve(*pTrimmedString, str.bytes - iter.position));
  pTrimmedString->text[0] = '\0';

  if (charOffset < str.count - 1)
  {
    pTrimmedString->bytes = str.bytes - iter.position;
    pTrimmedString->count = str.count - charOffset;
    lsMemcpy(pTrimmedString->text, str.text + iter.position, str.bytes - iter.position);
  }

epilogue:
  return result;
}

lsResult string_TrimEnd(const string &str, const utf32_t trimmedChar, _Out_ string *pTrimmedString)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pTrimmedString == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(str.hasFailed || trimmedChar == 0, lsR_InvalidParameter);

  utf8::utf8_iter iter;
  utf8::utf8_initEx(&iter, str.text, str.bytes);

  {
    size_t firstMatchingChar = 0;
    size_t firstMatchingByte = 0;

    for (size_t charOffset = 0; charOffset < str.count - 1; charOffset++)
    {
      LS_ERROR_IF(1 != utf8::utf8_next(&iter), lsR_InternalError);

      if (iter.codepoint != trimmedChar)
      {
        firstMatchingChar = charOffset + 1;
        firstMatchingByte = iter.position + iter.size;
      }
    }

    LS_ERROR_CHECK(string_Create(pTrimmedString, ""));
    LS_ERROR_CHECK(string_Reserve(*pTrimmedString, firstMatchingByte + 1));
    pTrimmedString->text[0] = '\0';

    if (firstMatchingByte > 0)
    {
      pTrimmedString->bytes = firstMatchingByte + 1;
      pTrimmedString->count = firstMatchingChar + 1;
      lsMemcpy(pTrimmedString->text, str.text, firstMatchingByte);
      pTrimmedString->text[firstMatchingByte] = '\0';
    }
  }

epilogue:
  return result;
}

lsResult string_RemoveChar(const string &str, const utf32_t remove, _Out_ string *pResult)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pResult == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(str.hasFailed || remove == 0, lsR_InvalidParameter);

  LS_ERROR_CHECK(string_Create(pResult, ""));
  LS_ERROR_CHECK(string_Reserve(*pResult, str.bytes));
  pResult->text[0] = '\0';

  {
    bool lastWasMatch = false;
    size_t firstNoMatchByte = 0;
    size_t destinationOffset = 0;
    size_t destinationCharCount = 0;
    size_t destinationCharSize = 0;

    utf8::utf8_iter iter;
    utf8::utf8_initEx(&iter, str.text, str.bytes);

    for (size_t charOffset = 0; charOffset < str.count - 1; charOffset++)
    {
      LS_ERROR_IF(1 != utf8::utf8_next(&iter), lsR_InternalError);

      if (iter.codepoint == remove)
      {
        const size_t size = iter.position - firstNoMatchByte;

        if (!lastWasMatch && size > 0)
        {
          lsMemcpy(pResult->text + destinationOffset, str.text + firstNoMatchByte, size);
          destinationOffset += size;
        }

        firstNoMatchByte = iter.position + iter.size;
        lastWasMatch = true;
      }
      else
      {
        lastWasMatch = false;
        ++destinationCharCount;
        destinationCharSize += iter.size;
      }
    }

    if (!lastWasMatch)
    {
      const size_t size = iter.position - firstNoMatchByte;
      lsMemcpy(pResult->text + destinationOffset, str.text + firstNoMatchByte, size + iter.size);
      destinationOffset += size;
    }

    pResult->text[destinationOffset] = '\0';
    pResult->count = destinationCharCount + 1;
    pResult->bytes = destinationCharSize + 1;
  }

epilogue:
  return result;
}

lsResult string_RemoveString(const string &str, const string &remove, _Out_ string *pResult)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  LS_ERROR_IF(pResult == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(str.hasFailed || remove.hasFailed, lsR_InvalidParameter);

  if (remove.bytes <= 1 || str.bytes <= 1 || remove.count > str.count)
  {
    LS_ERROR_CHECK(string_Create(pResult, str));
    goto epilogue; // success!
  }

  LS_ERROR_CHECK(string_Create(pResult, ""));
  LS_ERROR_CHECK(string_Reserve(*pResult, str.bytes));

  LS_ERROR_CHECK(lsAllocZero(&pKmp, remove.count - 1));
  LS_ERROR_CHECK(lsAlloc(&pChars, remove.count - 1));

  string_GetKmp(remove, pChars, pKmp);

  {
    size_t charOffsetAfter, byteOffsetAfter;
    size_t charOffset = 0;
    size_t byteOffset = 0;
    size_t destinationOffset = 0;
    size_t destinationNotBytes = 0;
    size_t destinationNotCount = 0;
    bool contained = false;

    while (charOffset < str.count - 1)
    {
      string_FindNextWithKMP(str, charOffset, byteOffset, pChars, remove.count - 1, pKmp, &charOffsetAfter, &byteOffsetAfter, &contained);

      if (!contained)
      {
        const size_t size = str.bytes - 1 - byteOffset;
        lsMemcpy(pResult->text + destinationOffset, str.text + byteOffset, size);
        destinationOffset += size;

        break;
      }
      else if (byteOffsetAfter - (remove.bytes - 1) != byteOffset)
      {
        const size_t size = byteOffsetAfter - (remove.bytes - 1) - byteOffset;
        lsMemcpy(pResult->text + destinationOffset, str.text + byteOffset, size);
        destinationOffset += size;
      }

      charOffset = charOffsetAfter;
      byteOffset = byteOffsetAfter;

      destinationNotBytes += remove.bytes - 1;
      destinationNotCount += remove.count - 1;
    }

    pResult->text[destinationOffset] = '\0';
    pResult->bytes = str.bytes - destinationNotBytes;
    pResult->count = str.count - destinationNotCount;
  }

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

lsResult string_Replace(const string &str, const string &replace, const string &with, _Out_ string *pResult)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  LS_ERROR_IF(pResult == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(str.hasFailed || replace.hasFailed || with.hasFailed, lsR_InvalidParameter);

  if (replace.bytes <= 1 || str.bytes <= 1 || replace.count > str.count)
  {
    LS_ERROR_CHECK(string_Create(pResult, str));
    goto epilogue; // success!
  }

  LS_ERROR_CHECK(string_Create(pResult, ""));
  LS_ERROR_CHECK(string_Reserve(*pResult, str.bytes));

  LS_ERROR_CHECK(lsAllocZero(&pKmp, replace.count - 1));
  LS_ERROR_CHECK(lsAlloc(&pChars, replace.count - 1));

  string_GetKmp(replace, pChars, pKmp);

  {
    size_t charOffsetAfter, byteOffsetAfter;
    size_t charOffset = 0;
    size_t byteOffset = 0;
    size_t destinationOffset = 0;
    size_t destinationNotBytes = 0;
    size_t destinationNotCount = 0;
    size_t destinationAddedBytes = 0;
    size_t destinationAddedCount = 0;
    bool contained = false;

    while (charOffset < str.count - 1)
    {
      string_FindNextWithKMP(str, charOffset, byteOffset, pChars, replace.count - 1, pKmp, &charOffsetAfter, &byteOffsetAfter, &contained);

      if (!contained)
      {
        const size_t size = str.bytes - 1 - byteOffset;

        if (destinationOffset + size >= pResult->capacity)
          LS_ERROR_CHECK(string_Reserve(*pResult, lsMax(pResult->capacity * 2, pResult->capacity + destinationOffset + size)));

        lsMemcpy(pResult->text + destinationOffset, str.text + byteOffset, size);
        destinationOffset += size;

        break;
      }
      else if (byteOffsetAfter - (replace.bytes - 1) != byteOffset)
      {
        const size_t size = byteOffsetAfter - (replace.bytes - 1) - byteOffset;

        if (destinationOffset + size + with.bytes >= pResult->capacity)
          LS_ERROR_CHECK(string_Reserve(*pResult, lsMax(pResult->capacity * 2, pResult->capacity + destinationOffset + size + with.bytes)));

        lsMemcpy(pResult->text + destinationOffset, str.text + byteOffset, size);
        destinationOffset += size;

        lsMemcpy(pResult->text + destinationOffset, with.text, with.bytes - 1);
        destinationOffset += with.bytes - 1;

        destinationAddedBytes += with.bytes - 1;
        destinationAddedCount += with.count - 1;
      }
      else
      {
        if (destinationOffset + with.bytes >= pResult->capacity)
          LS_ERROR_CHECK(string_Reserve(*pResult, lsMax(pResult->capacity * 2, pResult->capacity + destinationOffset + with.bytes)));

        lsMemcpy(pResult->text + destinationOffset, with.text, with.bytes - 1);
        destinationOffset += with.bytes - 1;

        destinationAddedBytes += with.bytes - 1;
        destinationAddedCount += with.count - 1;
      }

      charOffset = charOffsetAfter;
      byteOffset = byteOffsetAfter;

      destinationNotBytes += replace.bytes - 1;
      destinationNotCount += replace.count - 1;
    }

    pResult->text[destinationOffset] = '\0';
    pResult->bytes = str.bytes - destinationNotBytes + destinationAddedBytes;
    pResult->count = str.count - destinationNotCount + destinationAddedCount;
  }

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

//////////////////////////////////////////////////////////////////////////

lsResult small_string_GetCount_Internal(IN const char *text, const size_t maxSize, _Out_ size_t *pCount, _Out_ size_t *pSize)
{
  lsResult result = lsR_Success;

  if (maxSize == 0)
  {
    *pCount = 0;
    *pSize = 0;
    goto epilogue;
  }

  {
    size_t count = 0;
    utf8::utf8_iter iter;
    utf8::utf8_initEx(&iter, text, maxSize);

    while (true)
    {
      if (1 != utf8::utf8_next(&iter))
        break;

      count++;
    }

    if (text[iter.position] == '\0')
    {
      *pCount = count + 1;
      *pSize = iter.position + 1;
    }
    else
    {
      *pCount = count;
      *pSize = iter.position;
    }
  }

  goto epilogue;
epilogue:
  return result;
}

bool small_string_StringsAreEqual_Internal(IN const char *textA, IN const char *textB, const size_t bytes, const size_t count)
{
  utf8::utf8_iter iterA, iterB;
  utf8::utf8_initEx(&iterA, textA, bytes);
  utf8::utf8_initEx(&iterB, textB, bytes);

  if (!count)
    return true;

  for (size_t i = 0; i < count - 1; ++i)
  {
    LS_DEBUG_ASSERT_TRUE(1 != utf8::utf8_next(&iterA));
    LS_DEBUG_ASSERT_TRUE(1 != utf8::utf8_next(&iterB));

    if (iterA.codepoint != iterB.codepoint)
      return false;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////

utf8_string_iterator::utf8_string_iterator(const char *string) :
  utf8_string_iterator(string, lsStringLength(string) + 1)
{ }

utf8_string_iterator::utf8_string_iterator(const char *string, const size_t bytes) :
  string(string),
  bytes(bytes)
{
  utf8::utf8_initEx(&iter, string, bytes);
}

utf8_string_iterator &utf8_string_iterator::begin()
{
  return *this;
}

utf8_string_iterator utf8_string_iterator::end()
{
  return *this;
}

bool utf8_string_iterator::operator!=(const utf8_string_iterator &)
{
  if (1 != utf8::utf8_next(&iter))
    return false;

  charCount++;

  return true;
}

utf8_string_iterator &utf8_string_iterator::operator++()
{
  return *this;
}

iterated_char utf8_string_iterator::operator*()
{
  return iterated_char(string + iter.position, (utf32_t)iter.codepoint, iter.size, charCount - 1, iter.position);
}

iterated_char::iterated_char(const char *character, const utf32_t codePoint, const size_t characterSize, const size_t index, const size_t offset) :
  character(character),
  characterSize(characterSize),
  index(index),
  codePoint(codePoint),
  offset(offset)
{ }

//////////////////////////////////////////////////////////////////////////

#include "testable.h"
REGISTER_TESTABLE_FILE(1)

DEFINE_TESTABLE(string_TestCreateEmpty)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, ""));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(str, &count));
  TESTABLE_ASSERT_EQUAL(count, 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(str, &size));
  TESTABLE_ASSERT_EQUAL(size, 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateNullptr)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (char *)nullptr));

  TESTABLE_ASSERT_EQUAL(string.c_str(), nullptr);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 0);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 0);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToNullptr)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "not nullptr"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (char *)nullptr));

  TESTABLE_ASSERT_EQUAL(string, "");

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateNullptrLength)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (char *)nullptr, 1337));

  TESTABLE_ASSERT_EQUAL(string.c_str(), nullptr);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 0);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 0);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToNullptrLength)
{
  lsResult result = lsR_Success;

  const char testText[] = "not nullptr";

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, testText, LS_ARRAYSIZE(testText)));

  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (char *)nullptr, 1337));

  TESTABLE_ASSERT_EQUAL(string, "");

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateNullptrWcharT)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (wchar_t *)nullptr));

  TESTABLE_ASSERT_EQUAL(string.c_str(), nullptr);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 0);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 0);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToNullptrWcharT)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "not nullptr"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (wchar_t *)nullptr));

  TESTABLE_ASSERT_EQUAL(string, "");

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateNullptrLengthWcharT)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (wchar_t *)nullptr, 1337));

  TESTABLE_ASSERT_EQUAL(string.c_str(), nullptr);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 0);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 0);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToNullptrLengthWcharT)
{
  lsResult result = lsR_Success;

  const char testText[] = "not nullptr";

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, testText, LS_ARRAYSIZE(testText)));

  TESTABLE_ASSERT_SUCCESS(string_Create(&string, (wchar_t *)nullptr, 1337));

  TESTABLE_ASSERT_EQUAL(string, "");

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateASCII)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "h"));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 2);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 2);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateMultipleASCII)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "a b c"));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 6);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 6);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateUTF8)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "🌵🦎🎅"));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 4);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateMixed)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "🌵🦎🎅testמⴲ"));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 2 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 2 + 3 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateLength)
{
  lsResult result = lsR_Success;

  const char text[] = "test123";

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, text, 4));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 5);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 5);

  TESTABLE_ASSERT_EQUAL('\0', string.c_str()[string.Size() - 1]);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateWideLength)
{
  lsResult result = lsR_Success;

  const wchar_t text[] = L"test123";

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, text, 4));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 5);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(string, &size));
  TESTABLE_ASSERT_EQUAL(size, 5);

  TESTABLE_ASSERT_EQUAL('\0', string.c_str()[string.Size() - 1]);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateFromWchar)
{
  lsResult result = lsR_Success;

  string string;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, L"Testמ"));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(string, &count));
  TESTABLE_ASSERT_EQUAL(count, 4 + 2 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateFrostring)
{
  lsResult result = lsR_Success;

  string stringA;
  string stringB;
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, stringA));

  TESTABLE_ASSERT_EQUAL(stringA.bytes, stringB.bytes);
  TESTABLE_ASSERT_EQUAL(stringA.count, stringB.count);
  TESTABLE_ASSERT_NOT_EQUAL((void *)stringA.text, (void *)stringB.text);
  TESTABLE_ASSERT_TRUE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, stringB));

  TESTABLE_ASSERT_EQUAL(stringA.bytes, stringB.bytes);
  TESTABLE_ASSERT_EQUAL(stringA.count, stringB.count);
  TESTABLE_ASSERT_NOT_EQUAL((void *)stringA.text, (void *)stringB.text);
  TESTABLE_ASSERT_TRUE(stringA == stringB);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAt)
{
  lsResult result = lsR_Success;

  string string;
  utf32_t c;
  TESTABLE_ASSERT_SUCCESS(string_Create(&string, "🌵🦎🎅testמⴲ"));

  c = lsToChar<4>("🌵");
  TESTABLE_ASSERT_EQUAL(c, string[0]);
  TESTABLE_ASSERT_NOT_EQUAL(c, string[1]);
  c = lsToChar<4>("🦎");
  TESTABLE_ASSERT_EQUAL(c, string[1]);
  c = lsToChar<4>("🎅");
  TESTABLE_ASSERT_EQUAL(c, string[2]);
  c = lsToChar<1>("t");
  TESTABLE_ASSERT_EQUAL(c, string[3]);
  c = lsToChar<1>("e");
  TESTABLE_ASSERT_EQUAL(c, string[4]);
  c = lsToChar<1>("s");
  TESTABLE_ASSERT_EQUAL(c, string[5]);
  c = lsToChar<1>("t");
  TESTABLE_ASSERT_EQUAL(c, string[6]);
  c = lsToChar<2>("מ");
  TESTABLE_ASSERT_EQUAL(c, string[7]);
  c = lsToChar<3>("ⴲ");
  TESTABLE_ASSERT_EQUAL(c, string[8]);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppend)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));

  TESTABLE_ASSERT_SUCCESS(string_Append(str, string("🦎T")));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(str, &count));
  TESTABLE_ASSERT_EQUAL(count, 4 + 5 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(str, &size));
  TESTABLE_ASSERT_EQUAL(size, 16 + 5 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendOperator)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));

  str += string("🦎T");
  TESTABLE_ASSERT_FALSE(str.hasFailed);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(str, &count));
  TESTABLE_ASSERT_EQUAL(count, 4 + 5 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(str, &size));
  TESTABLE_ASSERT_EQUAL(size, 16 + 5 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestConcatOperator)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));
  
  {
    string newString = str + string("🦎T");
    TESTABLE_ASSERT_FALSE(newString.hasFailed);

    size_t count;
    TESTABLE_ASSERT_SUCCESS(string_GetCount(newString, &count));
    TESTABLE_ASSERT_EQUAL(count, 4 + 5 + 1);

    size_t size;
    TESTABLE_ASSERT_SUCCESS(string_GetByteSize(newString, &size));
    TESTABLE_ASSERT_EQUAL(size, 16 + 5 + 1);
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendEmpty)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));

  TESTABLE_ASSERT_SUCCESS(string_Append(str, string()));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(str, &count));
  TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(str, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendOperatorEmpty)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));

  str += string();
  TESTABLE_ASSERT_FALSE(str.hasFailed);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(str, &count));
  TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(str, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestConcatOperatorEmpty)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));
  
  {
    string newString = str + string();
    TESTABLE_ASSERT_FALSE(newString.hasFailed);
    
    size_t count;
    TESTABLE_ASSERT_SUCCESS(string_GetCount(newString, &count));
    TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

    size_t size;
    TESTABLE_ASSERT_SUCCESS(string_GetByteSize(newString, &size));
    TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendEmptyBase)
{
  lsResult result = lsR_Success;

  string str;
  string emptyString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));
  TESTABLE_ASSERT_SUCCESS(string_Append(emptyString, str));

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(emptyString, &count));
  TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(emptyString, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendOperatorEmptyBase)
{
  lsResult result = lsR_Success;

  string str;
  string emptyString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));
  emptyString += str;
  TESTABLE_ASSERT_FALSE(str.hasFailed);

  size_t count;
  TESTABLE_ASSERT_SUCCESS(string_GetCount(emptyString, &count));
  TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

  size_t size;
  TESTABLE_ASSERT_SUCCESS(string_GetByteSize(emptyString, &size));
  TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestConcatOperatorEmptyBase)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅test"));

  {
    string newString = string() + str;
    TESTABLE_ASSERT_FALSE(newString.hasFailed);

    size_t count;
    TESTABLE_ASSERT_SUCCESS(string_GetCount(newString, &count));
    TESTABLE_ASSERT_EQUAL(count, 3 + 4 + 1);

    size_t size;
    TESTABLE_ASSERT_SUCCESS(string_GetByteSize(newString, &size));
    TESTABLE_ASSERT_EQUAL(size, 12 + 4 + 1);
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestWstringCompare)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅testמⴲx"));

  wchar_t wstring[1024];
  TESTABLE_ASSERT_SUCCESS(string_ToWideString(str, wstring, LS_ARRAYSIZE(wstring)));

  {
    string string2;
    TESTABLE_ASSERT_SUCCESS(string_Create(&string2, wstring));

    TESTABLE_ASSERT_EQUAL(str, string2);
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSubstring)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅testמⴲx"));

  {
    string subString;
    TESTABLE_ASSERT_SUCCESS(string_Substring(str, &subString, 2, 7));

    {
      string comparisonString;
      TESTABLE_ASSERT_SUCCESS(string_Create(&comparisonString, "🎅testמⴲ"));

      TESTABLE_ASSERT_EQUAL(comparisonString.count, subString.count);
      TESTABLE_ASSERT_EQUAL(comparisonString.bytes, subString.bytes);

      for (size_t i = 0; i < comparisonString.count; i++)
        TESTABLE_ASSERT_EQUAL(comparisonString[i], subString[i]);

      TESTABLE_ASSERT_SUCCESS(string_Substring(str, &subString, 1));
      comparisonString = string("🦎") + comparisonString + "x";

      TESTABLE_ASSERT_EQUAL(comparisonString.count, subString.count);
      TESTABLE_ASSERT_EQUAL(comparisonString.bytes, subString.bytes);

      for (size_t i = 0; i < comparisonString.count; i++)
        TESTABLE_ASSERT_EQUAL(comparisonString[i], subString[i]);
    }
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestEquals)
{
  lsResult result = lsR_Success;

  string stringA;
  string stringB;
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, ""));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, ""));

  TESTABLE_ASSERT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_FALSE(stringA != stringB);
  TESTABLE_ASSERT_TRUE(stringA == stringB);

  bool equal;
  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_TRUE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲx"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🌵🦎🎅testמⴲx"));

  TESTABLE_ASSERT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_FALSE(stringA != stringB);
  TESTABLE_ASSERT_TRUE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_TRUE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅 estמⴲx"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🌵🦎🎅testמⴲx"));

  TESTABLE_ASSERT_NOT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_TRUE(stringA != stringB);
  TESTABLE_ASSERT_FALSE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_FALSE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🌵🦎🎅testמⴲx"));

  TESTABLE_ASSERT_NOT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_TRUE(stringA != stringB);
  TESTABLE_ASSERT_FALSE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_FALSE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲy"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🌵🦎🎅testמⴲx"));

  TESTABLE_ASSERT_NOT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_TRUE(stringA != stringB);
  TESTABLE_ASSERT_FALSE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_FALSE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲ🌵מk"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🌵🦎🎅testמⴲמ🌵k"));

  TESTABLE_ASSERT_NOT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_TRUE(stringA != stringB);
  TESTABLE_ASSERT_FALSE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_FALSE(equal);

  TESTABLE_ASSERT_SUCCESS(string_Create(&stringA, "🌵🦎🎅testמⴲמ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&stringB, "🦎🌵🎅testמⴲמ"));

  TESTABLE_ASSERT_NOT_EQUAL(stringA, stringB);
  TESTABLE_ASSERT_TRUE(stringA != stringB);
  TESTABLE_ASSERT_FALSE(stringA == stringB);

  TESTABLE_ASSERT_SUCCESS(string_Equals(stringA, stringB, &equal));
  TESTABLE_ASSERT_FALSE(equal);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestToDirectoryPath)
{
  lsResult result = lsR_Success;

  string str;
  string path;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "C:/Some Folder🌵//Path/\\p"));
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "C:\\Some Folder🌵\\Path\\p\\");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "../Some Folder//Path\\\\p/"));
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "..\\Some Folder\\Path\\p\\");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "/Some Folder//Path/\\/p\\"));
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "\\Some Folder\\Path\\p\\");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "test"));
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "test\\");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\\\\NetworkPath//Path/\\/p\\"));
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "\\\\NetworkPath\\Path\\p\\");

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestToFilePath)
{
  lsResult result = lsR_Success;

  string str;
  string path;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "C:/Some Folder//Path/\\p"));
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "C:\\Some Folder\\Path\\p");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "../Some Folder//Path\\\\p"));
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "..\\Some Folder\\Path\\p");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "/Some Folder//Path/\\/p"));
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "\\Some Folder\\Path\\p");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "test"));
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "test");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\\\\NetworkPath//Path/\\/p"));
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "\\\\NetworkPath\\Path\\p");

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestToDirectoryPathEmpty)
{
  lsResult result = lsR_Success;

  string str, path;
  TESTABLE_ASSERT_SUCCESS(string_ToDirectoryPath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "");

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestToFilePathEmpty)
{
  lsResult result = lsR_Success;

  string str, path;
  TESTABLE_ASSERT_SUCCESS(string_ToFilePath(&path, str));

  TESTABLE_ASSERT_EQUAL(path, "");

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendEqualsStringFunction)
{
  lsResult result = lsR_Success;

  string str;
  string appendedString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, ""));
  TESTABLE_ASSERT_SUCCESS(string_Create(&appendedString, "test string 1 2 3"));

  for (size_t i = 0; i < 1024; i++)
    TESTABLE_ASSERT_SUCCESS(string_Append(str, appendedString));

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendEqualsStringOperator)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, ""));

  for (size_t i = 0; i < 1024; i++)
    str += "test string 1 2 3";

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendStringOperator)
{
  lsResult result = lsR_Success;

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, ""));

  for (size_t i = 0; i < 8; i++)
    str += str + "test string 1 2 3" + str + "test";

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestIterate)
{
  lsResult result = lsR_Success;

  size_t charSize[] = { 4, 4, 4, 1, 1, 1, 1, 2, 3, 1 };
  size_t count = 0;

  string str;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "🌵🦎🎅testמⴲx"));

  for (auto &&_char : str.begin())
  {
    if (count == 0)
      TESTABLE_ASSERT_EQUAL(*(uint32_t *)str.text, *(uint32_t *)_char.character);

    TESTABLE_ASSERT_EQUAL(_char.index, count);
    TESTABLE_ASSERT_EQUAL(_char.characterSize, charSize[count]);
    count++;
  }

  TESTABLE_ASSERT_EQUAL(str.Count() - 1, count); // we don't care about the '\0' character.

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateFromEmpty)
{
  lsResult result = lsR_Success;

  string emptyString, str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, emptyString));

  TESTABLE_ASSERT_EQUAL(str.bytes, 0);
  TESTABLE_ASSERT_EQUAL(str.count, 0);
  TESTABLE_ASSERT_EQUAL(str.text, nullptr);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateFromNowEmpty)
{
  lsResult result = lsR_Success;

  string emptyString;
  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyString, "THIS IS NOT EMPTY"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyString, (char *)nullptr));
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, emptyString));

  TESTABLE_ASSERT_EQUAL(str.bytes, 0);
  TESTABLE_ASSERT_EQUAL(str.count, 0);
  TESTABLE_ASSERT_EQUAL(str.text, nullptr);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestCreateFromTooBigInvalidUTF8)
{
  lsResult result = lsR_Success;

  char text[] = { '\xf0', '\x9f', '\x8c', '\xb5', 'X', '\0', 'A', '\x9D', '\x8C', '\x86', '\x20' };

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, text));

  TESTABLE_ASSERT_EQUAL(str.bytes, 6);
  TESTABLE_ASSERT_EQUAL(str.count, 3);

  str = "";
  TESTABLE_ASSERT_EQUAL(str.bytes, 1);
  TESTABLE_ASSERT_EQUAL(str.count, 1);

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, text, LS_ARRAYSIZE(text)));

  TESTABLE_ASSERT_EQUAL(str.bytes, 6);
  TESTABLE_ASSERT_EQUAL(str.count, 3);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestAppendWCharT)
{
  lsResult result = lsR_Success;

  const char compString[] = "testString, blah, blah" "horrible WCharT String";

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "testString, blah, blah"));

  str = str + L"horrible WCharT String";

  TESTABLE_ASSERT_EQUAL(str, string(compString));
  TESTABLE_ASSERT_EQUAL(strlen(str.c_str()), strlen(compString));

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToWCharT)
{
  lsResult result = lsR_Success;

  const char compString[] = "horrible WCharT String";

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "this is a very long testString to allocate a sufficient amount of memory for the wchar_t string"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, L"horrible WCharT String"));

  TESTABLE_ASSERT_EQUAL(str, string(compString));
  TESTABLE_ASSERT_EQUAL(strlen(str.c_str()), strlen(compString));

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, L"horrible WCharT String"));

  TESTABLE_ASSERT_EQUAL(str, string(compString));
  TESTABLE_ASSERT_EQUAL(strlen(str.c_str()), strlen(compString));

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestSetToWCharTLong)
{
  lsResult result = lsR_Success;

  const char compString[] = "\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6\xE2\x80\xA6";

  string str;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, ""));

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, L"\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026\x2026"));

  TESTABLE_ASSERT_EQUAL(str, string(compString));
  TESTABLE_ASSERT_EQUAL(strlen(str.c_str()), strlen(compString));
  TESTABLE_ASSERT_EQUAL(28, str.count);
  TESTABLE_ASSERT_EQUAL(27 * 3 + 1, str.bytes);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestWstringLargerBufferSizeCreate)
{
  lsResult result = lsR_Success;

  wchar_t testString[10] = { 'b', 'i', 'n', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
  string string;

  TESTABLE_ASSERT_SUCCESS(string_Create(&string, testString, LS_ARRAYSIZE(testString)));

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestStartsWith)
{
  lsResult result = lsR_Success;

  string testString;
  string empty;
  string emptyInit;
  string invalid;
  string notTheStart;
  string theStart;
  string invalidStart = "\xF0\x93\x83\xA0\xF0\x93\x83";

  TESTABLE_ASSERT_SUCCESS(string_Create(&testString, "\xF0\x93\x83\xA0\xF0\x93\x83\xA1\xF0\x93\x83\xA2\xF0\x93\x83\xA3\xF0\x93\x83\xA4\xF0\x93\x83\xA5\xF0\x93\x83\xA6\xF0\x93\x83\xA7\xF0\x93\x83\xA8\xF0\x93\x83\xA9\xF0\x93\x83\xAA\xF0\x93\x83\xAB\xF0\x93\x83\xAC\xF0\x93\x83\xAD"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));
  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notTheStart, "\xe1\x86\xb0\xe1\x86\xb1\xe1\x86\xb2\xe1\x86\xb3\xe1\x86\xb4"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&theStart, "\xf0\x93\x83\xa0\xf0\x93\x83\xa1\xf0\x93\x83\xa2\xf0\x93\x83\xa3\xf0\x93\x83\xa4\xf0\x93\x83\xa5"));

  bool startsWith;
  TESTABLE_ASSERT_SUCCESS(string_StartsWith(testString, empty, &startsWith));
  TESTABLE_ASSERT_TRUE(startsWith);
  TESTABLE_ASSERT_TRUE(testString.StartsWith(empty));

  TESTABLE_ASSERT_SUCCESS(string_StartsWith(testString, emptyInit, &startsWith));
  TESTABLE_ASSERT_TRUE(startsWith);

  TESTABLE_ASSERT_SUCCESS(string_StartsWith(empty, testString, &startsWith));
  TESTABLE_ASSERT_FALSE(startsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_StartsWith(testString, invalid, &startsWith));
  TESTABLE_ASSERT_FALSE(startsWith);
  TESTABLE_ASSERT_FALSE(testString.StartsWith(invalid));

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_StartsWith(invalid, testString, &startsWith));
  TESTABLE_ASSERT_FALSE(startsWith);

  TESTABLE_ASSERT_SUCCESS(string_StartsWith(testString, testString, &startsWith));
  TESTABLE_ASSERT_TRUE(startsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_StartsWith(invalid, invalidStart, &startsWith));
  TESTABLE_ASSERT_FALSE(startsWith);

  TESTABLE_ASSERT_SUCCESS(string_StartsWith(testString, notTheStart, &startsWith));
  TESTABLE_ASSERT_FALSE(startsWith);

  TESTABLE_ASSERT_SUCCESS(string_StartsWith(testString, theStart, &startsWith));
  TESTABLE_ASSERT_TRUE(startsWith);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestEndsWithASCII)
{
  lsResult result = lsR_Success;

  string testString;
  string empty;
  string emptyInit;
  string invalid;
  string notTheEnd;
  string theEnd;
  string invalidEnd = "abqabxabyab";

  TESTABLE_ASSERT_SUCCESS(string_Create(&testString, "abAabBabCabDabEabFabGabHabqabxabyabI"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));

  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notTheEnd, "JKLJKMJKNJKOJKP"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&theEnd, "abqabxabyabI"));

  bool endsWith;
  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, empty, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);
  TESTABLE_ASSERT_TRUE(testString.EndsWith(empty));

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, emptyInit, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(empty, testString, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(testString, invalid, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);
  TESTABLE_ASSERT_FALSE(testString.EndsWith(invalid));

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(invalid, testString, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, testString, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(invalid, invalidEnd, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, notTheEnd, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, theEnd, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestEndsWithUTF8)
{
  lsResult result = lsR_Success;

  string testString;
  string empty;
  string emptyInit;
  string invalid;
  string notTheEnd;
  string theEnd;
  string invalidEnd = "\xe0\xa6\x9c\xe0\xa6\x9d\xe0\xa6\x9e\xe0\xa6";

  TESTABLE_ASSERT_SUCCESS(string_Create(&testString, "\xe0\xa6\x93\xe0\xa6\x94\xe0\xa6\x95\xe0\xa6\x96\xe0\xa6\x98\xe0\xa6\x99\xe0\xa6\x9a\xe0\xa6\x9b\xe0\xa6\x9c\xe0\xa6\x9d\xe0\xa6\x9e\xe0\xa6\x9f"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));

  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notTheEnd, "\xe1\x86\xb0\xe1\x86\xb1\xe1\x86\xb2\xe1\x86\xb3\xe1\x86\xb4"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&theEnd, "\xe0\xa6\x9c\xe0\xa6\x9d\xe0\xa6\x9e\xe0\xa6\x9f"));

  bool endsWith;
  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, empty, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);
  TESTABLE_ASSERT_TRUE(testString.EndsWith(empty));

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, emptyInit, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(empty, testString, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(testString, invalid, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);
  TESTABLE_ASSERT_FALSE(testString.EndsWith(invalid));

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(invalid, testString, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, testString, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_EndsWith(invalid, invalidEnd, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, notTheEnd, &endsWith));
  TESTABLE_ASSERT_FALSE(endsWith);

  TESTABLE_ASSERT_SUCCESS(string_EndsWith(testString, theEnd, &endsWith));
  TESTABLE_ASSERT_TRUE(endsWith);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestKmpAscii)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  size_t kmp[] = { 0, 0, 0, 1, 2, 3, 4, 1, 2, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 5, 6, 7, 5, 6, 0, 1, 0, 1, 2, 1, 2, 1, 0 };

  string testString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&testString, "abcabcaabbccabcabcabcabcbacababac"));

  TESTABLE_ASSERT_SUCCESS(lsAllocZero(&pKmp, testString.count - 1));
  TESTABLE_ASSERT_SUCCESS(lsAlloc(&pChars, testString.count - 1));

  string_GetKmp(testString, pChars, pKmp);

  for (size_t i = 0; i < LS_ARRAYSIZE(kmp); i++)
    TESTABLE_ASSERT_EQUAL(kmp[i], pKmp[i]);

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

DEFINE_TESTABLE(string_TestKmpUnicode)
{
  lsResult result = lsR_Success;

  size_t *pKmp = nullptr;
  utf32_t *pChars = nullptr;

  size_t kmp[] = { 0, 0, 0, 1, 2, 3, 4, 1, 2, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 5, 6, 7, 5, 6, 0, 1, 0, 1, 2, 1, 2, 1, 0 };

  string testString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&testString, "Я中CЯ中CЯЯ中中CCЯ中CЯ中CЯ中CЯ中C中ЯCЯ中Я中ЯC"));

  TESTABLE_ASSERT_SUCCESS(lsAllocZero(&pKmp, testString.count - 1));
  TESTABLE_ASSERT_SUCCESS(lsAlloc(&pChars, testString.count - 1));

  string_GetKmp(testString, pChars, pKmp);

  for (size_t i = 0; i < LS_ARRAYSIZE(kmp); i++)
    TESTABLE_ASSERT_EQUAL(kmp[i], pKmp[i]);

epilogue:
  lsFreePtr(&pKmp);
  lsFreePtr(&pChars);

  return result;
}

DEFINE_TESTABLE(string_TestContainsAscii)
{
  lsResult result = lsR_Success;

  string str;
  string empty;
  string emptyInit;
  string invalid;
  string notContained;
  string notContainedChar;
  string contained;
  string start;
  string end;
  string startChar;
  string endChar;
  string midChar;
  bool isContained;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "ABCABCABCAAABBBCCCABCABCABCAACCBBBAABCABCBABCBBACBAAAABBABABBCCX"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));

  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContained, "CAACCBBBAABCABAAAA"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContainedChar, "V"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&contained, "CAACCBBBAABCAB"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&start, "ABCABCABCAAABB"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&end, "ABBCC"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&startChar, "A"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&endChar, "X"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&midChar, "C"));

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, empty, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, emptyInit, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_Contains(str, invalid, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_Contains(invalid, str, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, notContained, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, notContainedChar, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, contained, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, start, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, end, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, startChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, endChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, midChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestContainsUnicode)
{
  lsResult result = lsR_Success;

  string str;
  string empty;
  string emptyInit;
  string invalid;
  string notContained;
  string notContainedChar;
  string contained;
  string start;
  string end;
  string startChar;
  string endChar;
  string midChar;
  bool isContained = false;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\xe0\xbc\x80\xe0\xbc\x81\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85\xe0\xbc\x86\xe0\xbc\x87\xe0\xbc\x88\xe0\xbc\x89\xe0\xbc\x8a\xe0\xbc\x95\xe0\xbc\x96\xe0\xbc\x97\xe0\xbc\x98\xe0\xbc\x99\xe0\xbf\x90\xe1\x8f\xa0\xe1\x8f\xa1\xe1\x8f\xa2\xe1\x8f\xa3\xe1\x8f\xa4\xe1\x8f\xa5\xe1\x8f\xa6\xe1\x8f\xa7\xe1\x8f\xa8\xe1\x8f\xa9\xe1\x8f\xaa\xe1\x8f\xab\xe1\x8f\xac\xe1\x8f\xad\xe1\x8f\xae\xe1\x8f\xaf\xe0\xb8\xaa\xe0\xb8\xa7\xe0\xb8\xb1\xe0\xb8\xaa\xe0\xb8\x94\xe0\xb8\xb5\xe0\xb8\x8a\xe0\xb8\xb2\xe0\xb8\xa7\xe0\xb9\x82\xe0\xb8\xa5\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));

  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContained, "\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85\xe1\x8f\xa8"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContainedChar, "V"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&contained, "\xe0\xbc\x96\xe0\xbc\x97\xe0\xbc\x98\xe0\xbc\x99\xe0\xbf\x90\xe1\x8f\xa0"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&start, "\xe0\xbc\x80\xe0\xbc\x81\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&end, "\xe0\xb8\xa7\xe0\xb9\x82\xe0\xb8\xa5\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&startChar, "\xe0\xbc\x80"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&endChar, "\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&midChar, "\xe1\x8f\xa4"));

  TESTABLE_ASSERT_SUCCESS(string_Contains(str, empty, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, emptyInit, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_Contains(str, invalid, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_Contains(invalid, str, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, notContained, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, notContainedChar, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, contained, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, start, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, end, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, startChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, endChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_Contains(str, midChar, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestFindFirst)
{
  lsResult result = lsR_Success;

  string str;
  string empty;
  string emptyInit;
  string invalid;
  string notContained;
  string notContainedChar;
  string contained;
  string start;
  string end;
  string startChar;
  string endChar;
  string midChar;
  bool isContained = false;
  size_t index = 0;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "abcdef"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&contained, "cde"));

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, contained, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < contained.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], contained[i]);

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\xe0\xbc\x80\xe0\xbc\x81\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85\xe0\xbc\x86\xe0\xbc\x87\xe0\xbc\x88\xe0\xbc\x89\xe0\xbc\x8a\xe0\xbc\x95\xe0\xbc\x96\xe0\xbc\x97\xe0\xbc\x98\xe0\xbc\x99\xe0\xbf\x90\xe1\x8f\xa0\xe1\x8f\xa1\xe1\x8f\xa2\xe1\x8f\xa3\xe1\x8f\xa4\xe1\x8f\xa5\xe1\x8f\xa6\xe1\x8f\xa7\xe1\x8f\xa8\xe1\x8f\xa9\xe1\x8f\xaa\xe1\x8f\xab\xe1\x8f\xac\xe1\x8f\xad\xe1\x8f\xae\xe1\x8f\xaf\xe0\xb8\xaa\xe0\xb8\xa7\xe0\xb8\xb1\xe0\xb8\xaa\xe0\xb8\x94\xe0\xb8\xb5\xe0\xb8\x8a\xe0\xb8\xb2\xe0\xb8\xa7\xe0\xb9\x82\xe0\xb8\xa5\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&emptyInit, ""));

  invalid.hasFailed = true;

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContained, "\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85\xe1\x8f\xa8"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&notContainedChar, "V"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&contained, "\xe0\xbc\x96\xe0\xbc\x97\xe0\xbc\x98\xe0\xbc\x99\xe0\xbf\x90\xe1\x8f\xa0"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&start, "\xe0\xbc\x80\xe0\xbc\x81\xe0\xbc\x82\xe0\xbc\x83\xe0\xbc\x84\xe0\xbc\x85"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&end, "\xe0\xb8\xa7\xe0\xb9\x82\xe0\xb8\xa5\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&startChar, "\xe0\xbc\x80"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&endChar, "\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_Create(&midChar, "\xe1\x8f\xa4"));

  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, empty, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);
  TESTABLE_ASSERT_EQUAL(0, index);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, emptyInit, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);
  TESTABLE_ASSERT_EQUAL(0, index);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_FindFirst(str, invalid, &index, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_EQUAL(lsR_InvalidParameter, string_FindFirst(invalid, str, &index, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, notContained, &index, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = true;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, notContainedChar, &index, &isContained));
  TESTABLE_ASSERT_FALSE(isContained);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, contained, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < contained.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], contained[i]);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, start, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < start.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], start[i]);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, end, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < end.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], end[i]);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, startChar, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < startChar.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], startChar[i]);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, endChar, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < endChar.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], endChar[i]);

  isContained = false;
  TESTABLE_ASSERT_SUCCESS(string_FindFirst(str, midChar, &index, &isContained));
  TESTABLE_ASSERT_TRUE(isContained);

  for (size_t i = 0; i < midChar.count - 1; i++)
    TESTABLE_ASSERT_EQUAL(str[index + i], midChar[i]);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestTrimStart)
{
  lsResult result = lsR_Success;

  utf32_t trimmedChar = lsToChar<2>(" ");
  utf32_t nonTrimmableChar = lsToChar<2>("\xe0\xbc\x82");
  char res[] = "\xe1\x8f\xa4 b c    d e \xe0\xb8\x81  ";

  string str;
  string trimmedString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "     \xe1\x8f\xa4 b c    d e \xe0\xb8\x81  "));

  TESTABLE_ASSERT_SUCCESS(string_TrimStart(str, trimmedChar, &trimmedString));
  TESTABLE_ASSERT_EQUAL(trimmedString, res);

  TESTABLE_ASSERT_SUCCESS(string_TrimStart(str, nonTrimmableChar, &trimmedString));
  TESTABLE_ASSERT_EQUAL(str, trimmedString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestTrimEnd)
{
  lsResult result = lsR_Success;

  utf32_t trimmedChar = lsToChar<2>(" ");
  utf32_t nonTrimmableChar = lsToChar<4>("\xe0\xbc\x82");
  char res[] = "     \xe1\x8f\xa4 b c    d e \xe0\xb8\x81";

  string str;
  string trimmedString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "     abcdef  "));
  TESTABLE_ASSERT_SUCCESS(string_TrimEnd(str, trimmedChar, &trimmedString));
  TESTABLE_ASSERT_EQUAL(trimmedString, "     abcdef");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "     \xe1\x8f\xa4 b c    d e \xe0\xb8\x81  "));

  TESTABLE_ASSERT_SUCCESS(string_TrimEnd(str, trimmedChar, &trimmedString));
  TESTABLE_ASSERT_EQUAL(trimmedString, res);

  TESTABLE_ASSERT_SUCCESS(string_TrimEnd(str, nonTrimmableChar, &trimmedString));
  TESTABLE_ASSERT_EQUAL(str, trimmedString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestRemoveCharASCII)
{
  lsResult result = lsR_Success;

  utf32_t replacedChar = lsToChar<4>("a");
  utf32_t replacedChar2 = lsToChar<4>("c");
  utf32_t nonReplaceableChar = lsToChar<2>(" ");
  char result1[] = "bebegbejicdfcdfhbecdf";
  char result2[] = "abeabegabejidfdfhabedf";

  string str;
  string resultString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "abeabegabejicdfcdfhabecdf"));

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, replacedChar, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, result1);

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, replacedChar2, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, result2);

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, nonReplaceableChar, &resultString));
  TESTABLE_ASSERT_EQUAL(str, resultString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestRemoveCharUTF8)
{
  lsResult result = lsR_Success;

  utf32_t replacedChar = lsToChar<4>("\xe1\x8f\xa4");
  utf32_t replacedChar2 = lsToChar<4>("\xe0\xb8\x81");
  utf32_t nonReplaceableChar = lsToChar<2>(" ");
  char result1[] = "\x62\x63\x64\xe0\xb8\x81\xe0\xb8\x81\x65\xe0\xb8\x81";
  char result2[] = "\xe1\x8f\xa4\xe1\x8f\xa4\x62\xe1\x8f\xa4\x63\x64\x65\xe1\x8f\xa4";

  string str;
  string resultString;
  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\xe1\x8f\xa4\xe1\x8f\xa4\x62\xe1\x8f\xa4\x63\x64\xe0\xb8\x81\xe0\xb8\x81\x65\xe1\x8f\xa4\xe0\xb8\x81"));

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, replacedChar, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, result1);

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, replacedChar2, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, result2);

  TESTABLE_ASSERT_SUCCESS(string_RemoveChar(str, nonReplaceableChar, &resultString));
  TESTABLE_ASSERT_EQUAL(str, resultString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestRemoveStringASCII)
{
  lsResult result = lsR_Success;

  string replace;
  string str;
  string resultString;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "abcdefg"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "cdef"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "abg");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "abc123def123123ghi123jkl"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "123"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "abcdefghijkl");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "bacdabcdacbacbdacbafacba"));

  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, str, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString.count, 1);
  TESTABLE_ASSERT_EQUAL(resultString.bytes, 1);
  TESTABLE_ASSERT_EQUAL(resultString.text[0], '\0');

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "acb"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "bacdabcddafa");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "facb"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "bacdabcdacbacbdacbaa");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "facba"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "bacdabcdacbacbdacba");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "bacdabcdacbacbdacbafacbba"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, str);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestRemoveStringUTF8)
{
  lsResult result = lsR_Success;

  string replace;
  string str;
  string resultString;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8C\xAC"));

  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, str, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString.count, 1);
  TESTABLE_ASSERT_EQUAL(resultString.bytes, 1);
  TESTABLE_ASSERT_EQUAL(resultString.text[0], '\0');

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\x80\xA9\xE1\x8C\xAC"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x8C\xAC");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8C\xAC"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8C\xAC");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\x80\xA9\xF0\x94\x93\x98\x76\xF0\x90\x8C\x86\x6c\xE1\x80\xA9\x64"));
  TESTABLE_ASSERT_SUCCESS(string_RemoveString(str, replace, &resultString));
  TESTABLE_ASSERT_EQUAL(str, resultString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestReplaceStringASCII)
{
  lsResult result = lsR_Success;

  string str, replace, with, resultString;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "abCDEfg"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "CDE"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&with, "-123-"));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "ab-123-fg");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "ABcdefcdefAB"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "AB"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&with, ""));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "cdefcdef");
  
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "cdef"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&with, "ab"));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "ABababAB");

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "cdebabafcdgacdacdibhjcdefcdbabhcdebafkklbjfcdbkhbbaa"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "khbbaa"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&with, ""));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "cdebabafcdgacdacdibhjcdefcdbabhcdebafkklbjfcdb");

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestReplaceStringUTF8)
{
  lsResult result = lsR_Success;

  string replace;
  string with;

  string str;
  string resultString;

  TESTABLE_ASSERT_SUCCESS(string_Create(&str, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8C\xAC"));

  TESTABLE_ASSERT_SUCCESS(string_Replace(str, str, str, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString.count, str.count);
  TESTABLE_ASSERT_EQUAL(resultString.bytes, str.bytes);
  TESTABLE_ASSERT_EQUAL(resultString, str);

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\x80\xA9\xE1\x8C\xAC"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&with, "\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86"));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8F\xAA\xE1\x8F\xAB\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xF0\x94\x93\x98\xF0\x90\x8C\x86\xE1\x8C\xAC");

  TESTABLE_ASSERT_SUCCESS(string_Create(&with, ""));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\x70\x70\x6C\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6F\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x8C\xAC");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\x65\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x8C\xAC\xE1\xA0\xAC\xE1\xA0\xAD\x72\xE1\x80\xA9\xF0\x94\x93\x98\x67"));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(resultString, "\xE1\xA0\xAC\xE1\xA0\xAD\x68\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\xE1\x8C\xAC\xE1\x80\xA9\xF0\x94\x93\x98\xE1\xA0\xAC\xE1\xA0\xAD\x68\xE1\x80\xA9\xE1\x8C\xAC\xF0\x90\x8C\x86\x70\x70\x6c\xE1\x80\xA9\x63\xF0\x90\x8C\x86\xE1\xA0\xAC\xE1\xA0\xAD\xE1\x80\xA9\x6f\xF0\x94\x93\x98\xE1\x80\xA9\xE1\x80\xA9\xE1\x8C\xAC\xE1\x8C\xAC");

  TESTABLE_ASSERT_SUCCESS(string_Create(&replace, "\xE1\x80\xA9\xF0\x94\x93\x98\x76\xF0\x90\x8C\x86\x6c\xE1\x80\xA9\x64"));
  TESTABLE_ASSERT_SUCCESS(string_Replace(str, replace, with, &resultString));
  TESTABLE_ASSERT_EQUAL(str, resultString);

epilogue:
  return result;
}

DEFINE_TESTABLE(string_TestFormatInteraction)
{
  lsResult result = lsR_Success;

  sformatState_ResetCulture();

  sformatState &fs = sformat_GetState();
  fs.textCapacity = 0;

  string resultString, cmp;
  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, 123, " and ", 234.567, " and ", true, "🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "123 and 234.567 and true🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, 0));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "0"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, -1234567));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "-1234567"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, 123));
  TESTABLE_ASSERT_SUCCESS(string_AppendFormat(resultString, " and ", 234.567, " and ", true, "🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "123 and 234.567 and true🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, 3.141));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "3.141"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

  TESTABLE_ASSERT_SUCCESS(string_Format(&resultString, 123));
  TESTABLE_ASSERT_SUCCESS(string_AppendFormat(resultString, " and "));
  TESTABLE_ASSERT_SUCCESS(string_AppendFormat(resultString, 234.567, " and "));
  TESTABLE_ASSERT_SUCCESS(string_AppendFormat(resultString, true, "🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_SUCCESS(string_Create(&cmp, "123 and 234.567 and true🌵🦎🎅testמⴲ"));
  TESTABLE_ASSERT_EQUAL(cmp, resultString);

epilogue:
  return result;
}
