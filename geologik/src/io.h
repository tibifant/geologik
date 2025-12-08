#pragma once

#include "core.h"

lsResult lsReadFileBytes(const char *filename, _Out_ uint8_t **ppData, const size_t elementSize, _Out_ size_t *pCount);

template <typename T>
lsResult lsReadFile(const char *filename, _Out_ T **ppData, _Out_ size_t *pCount)
{
  return lsReadFileBytes(filename, reinterpret_cast<uint8_t **>(ppData), sizeof(T), pCount);
}
