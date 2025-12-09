#include "gpuBuffer.h"

#include "GL/glew.h"

lsResult gpu_buffer_create(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);

  glCreateBuffers(1, &pBuffer->bufferId);

epilogue:
  return result;
}

lsResult gpu_buffer_set(gpu_buffer *pBuffer, uint8_t *pData /*nullptr for only intializing*/, const uint32_t bindingPoint = 0)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->bufferId, lsR_ResourceStateInvalid);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

  uint32_t accessType;

  switch (pBuffer->accessType)
  {
  case gbat_dynamic: accessType = GL_DYNAMIC_STORAGE_BIT; break;
  case gbat_read_only: accessType = GL_MAP_READ_BIT; break;
  case gbat_write_only: accessType = GL_MAP_WRITE_BIT; break;
  case gbat_persistent_stream: accessType = GL_MAP_PERSISTENT_BIT; break;
  case gbat_persistent_stream_immediate_update: accessType = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT; break;
  default: lsFail(); break; // not implemented.
  }

  glBufferStorage(GL_SHADER_STORAGE_BUFFER, pBuffer->size, pData, accessType); // in case we rather want to use a variable sized buffer we need to use `glBufferData` instead.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, pBuffer->bufferId);

  pBuffer->uploaded = true;

epilogue:
  if (pData != nullptr)
    lsFreePtr(&pData);

  return result;
}

lsResult gpu_buffer_get_data(gpu_buffer *pBuffer, _Out_ uint8_t **ppData, _Out_ size_t *pSize)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->bufferId, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pBuffer->uploaded, lsR_ResourceStateInvalid);

  LS_ERROR_IF(pBuffer->accessType == gbat_write_only, lsR_ResourceIncompatible);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  const uint8_t *pGpuData = reinterpret_cast<uint8_t *>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->size, GL_MAP_READ_BIT));
  
  LS_ERROR_IF(pGpuData == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(lsAlloc(ppData, pBuffer->size / sizeof(uint32_t)));
  lsMemcpy(*ppData, pGpuData, pBuffer->size / sizeof(uint32_t));

  *pSize = pBuffer->size;

epilogue:
  return result;
}

// TODO update data func for cases when buffer access type allows updating data

lsResult gpu_buffer_bind(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->bufferId, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pBuffer->uploaded, lsR_ResourceStateInvalid);
  
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

epilogue:
  return result;
}

void gpu_buffer_detroy(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  if (pBuffer == nullptr || !pBuffer->bufferId)
    return;

  glDeleteBuffers(1, &pBuffer->bufferId);

  pBuffer->bufferId = 0;
}
