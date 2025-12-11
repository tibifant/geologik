#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

enum gpu_buffer_acces_type
{
  gbat_dynamic, // GL_DYNAMIC_STORAGE_BIT
  gbat_read_only, // GL_MAP_READ_BIT
  gbat_write_only, // GL_MAP_WRITE_BIT
  gbat_persistent_stream, // GL_MAP_PERSISTENT_BIT
  gbat_persistent_stream_immediate_update, // GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
};

struct gpu_buffer
{
  uint32_t bufferId = 0; // this is actually a GLuint.
  
  size_t size; // in bytes
  gpu_buffer_acces_type accessType;
  bool uploaded = false;
};

//////////////////////////////////////////////////////////////////////////

lsResult gpu_buffer_create(gpu_buffer *pBuffer);
lsResult gpu_buffer_bind(gpu_buffer *pBuffer);
void gpu_buffer_detroy(gpu_buffer *pBuffer);

template<typename T>
lsResult gpu_buffer_set(gpu_buffer *pBuffer, const T *pData /*nullptr valid for intialization*/, const uint32_t bindingPoint = 0)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->bufferId, lsR_ResourceStateInvalid);

  LS_ERROR_CHECK(gpu_buffer_set(pBuffer, reinterpret_cast<const uint8_t *>(pData), bindingPoint)); // is this valid like this?

epilogue:
  return result;
}

template<typename T>
lsResult gpu_buffer_get_data(gpu_buffer *pBuffer, _Out_ T **ppData, _Out_ size_t *pCount)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(*ppData != nullptr, lsR_InvalidParameter);

  size_t size;
  LS_ERROR_CHECK(gpu_buffer_get_data(pBuffer, reinterpret_cast<uint8_t **>(ppData), &size));

  *pCount = size / sizeof(T);

epilogue:
  return result;
}
