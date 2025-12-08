#include "io.h"

lsResult lsReadFileBytes(const char *filename, uint8_t **ppData, const size_t elementSize, size_t *pCount)
{
  lsResult result = lsR_Success;

  FILE *pFile = nullptr;

  LS_ERROR_IF(filename == nullptr || ppData == nullptr || pCount == nullptr, lsR_ArgumentNull);

  pFile = fopen(filename, "rb");
  LS_ERROR_IF(pFile == nullptr, lsR_ResourceNotFound);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_END), lsR_IOFailure);

  const size_t length = ftell(pFile);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_SET), lsR_IOFailure);

  LS_ERROR_CHECK(lsAlloc(ppData, length + (elementSize > 2 ? 0 : elementSize)));

  if (elementSize <= 2)
    lsZeroMemory(&((*ppData)[length]), elementSize); // To zero terminate strings. This is out of bounds for all other data types anyways.

  const size_t readLength = fread(*ppData, 1, length, pFile);

  *pCount = readLength / elementSize;

epilogue:
  if (pFile != nullptr)
    fclose(pFile);

  return result;
}
