#include "gpuBuffer.h"

#include "GL/glew.h"

// create
lsResult gpu_buffer_create(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);

  glCreateBuffers(1, &pBuffer->bufferId); // I think this is correct for only one buffer

  // bind here?

  pBuffer->initialized = true;

epilogue:
  return result;
}

lsResult gpu_buffer_set(gpu_buffer *pBuffer, uint32_t *pData /*nullptr for only intializing*/, const uint32_t bindingPoint = 0)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->initialized, lsR_ResourceStateInvalid);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

  uint32_t bufferBit;

  switch (pBuffer->type)
  {
  case gbat_dynamic: bufferBit = GL_DYNAMIC_STORAGE_BIT; break;;
  case gbat_read_only: bufferBit = GL_MAP_READ_BIT; break;
  case gbat_write_only: bufferBit = GL_MAP_WRITE_BIT; break;
  case gbat_persistent_stream: bufferBit = GL_MAP_PERSISTENT_BIT; break;
  case gbat_persistent_stream_immediate_update: bufferBit = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT; break;
  default: lsFail(); break; // not implemented.
  }

  // TODO set access type in pBuffer so we know in `gpu_buffer_read` how and if we are allowed and able to read from it. but it can be set to somethign else again with set

  glBufferStorage(GL_SHADER_STORAGE_BUFFER, pBuffer->sizeInBytes, pData, bufferBit); // in case we rather want to use a variable sized buffer we need to use `glBufferData` instead.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, pBuffer->bufferId);

  pBuffer->uploaded = true;

epilogue:
  if (pData != nullptr)
    lsFreePtr(&pData);

  return result;
}

lsResult gpu_buffer_get_data(gpu_buffer *pBuffer, uint32_t *pData, const uint32_t size)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(pData != nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->initialized, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pBuffer->uploaded, lsR_ResourceStateInvalid);

  LS_ERROR_IF(pBuffer->type == gbat_write_only, lsR_ResourceIncompatible);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glFinish(); // i think this needed but only advised to be used when the gpu is finished anyways as it stops all gpu porcesses?

  uint32_t *pGpuData = (uint32_t *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, pBuffer->sizeInBytes, GL_MAP_READ_BIT); // this cast kinda feels illegal lol
  
  LS_ERROR_IF(pGpuData == nullptr, lsR_ArgumentNull);

  lsAlloc(&pData, pBuffer->sizeInBytes / sizeof(uint32_t));
  lsMemcpy(pData, pGpuData, pBuffer->sizeInBytes / sizeof(uint32_t));

epilogue:
  return result;
}

// TODO update data func for cases when buffer access type allows updating data

lsResult gpu_buffer_bind(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pBuffer->initialized, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pBuffer->uploaded, lsR_ResourceStateInvalid);
  
  // anything similar to activate texture?
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, pBuffer->bufferId);

epilogue:
  return result;
}

void gpu_buffer_detroy(gpu_buffer *pBuffer)
{
  lsResult result = lsR_Success;

  if (pBuffer == nullptr || !pBuffer->initialized)
    return;

  glDeleteBuffers(1, &pBuffer->bufferId);

  pBuffer->bufferId = 0;
}
