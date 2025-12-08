#pragma once

#include "core.h"
#include "shader.h"

#include <GL/glew.h>

struct vb_attribute
{
};

template <size_t TCount, const char *TAttributeName>
struct vb_attribute_float : vb_attribute
{
  static size_t getSingleSize() { return sizeof(float_t); };
  static const char *getAttributeName() { return TAttributeName; };
  static size_t getValuesPerBlock() { return TCount; };
  static size_t getDataSize() { return sizeof(float_t) * TCount; };

  static GLenum getDataType() { return GL_FLOAT; };
};

template <const char *TAttributeName>
struct vb_attribute_mat4 : vb_attribute
{
  static const char *getAttributeName() { return TAttributeName; };
  static size_t getDataSize() { return sizeof(float_t) * 4 * 4; };

  static GLenum getDataType() { return GL_FLOAT; };
};

template <typename ...Args>
struct vb_attributeQuery_internal
{
  static size_t getSize();

  static void setAttribute(shader *pShader, const size_t totalSize, const size_t offset, const bool instanced);
};

template <>
struct vb_attributeQuery_internal<>
{
  static size_t getSize() { return 0; };

  static void setAttribute(shader * /* pShader */, const size_t /* totalSize */, const size_t /* offset */, const bool /* instanced */) { };
};

template <typename T>
struct vb_attributeQuery_internal<T>
{
  static size_t getSize() { return T::getDataSize(); };

  static void setAttribute(shader *pShader, const size_t totalSize, const size_t offset, const bool instanced)
  {
    const uint32_t attributeIndex = shader_getAttributeIndex(pShader, T::getAttributeName());
    
    glEnableVertexAttribArray(attributeIndex);
    glVertexAttribPointer(attributeIndex, (GLint)T::getSingleSize(), T::getDataType(), GL_FALSE, (GLsizei)totalSize, (const void *)offset);

    if (instanced)
      glVertexAttribDivisor(attributeIndex, 1);
  }
};

template <const char *TAttributeName>
struct vb_attributeQuery_internal<vb_attribute_mat4<TAttributeName>>
{
  static size_t getSize() { return vb_attribute_mat4::getDataSize(); };

  static void setAttribute(shader *pShader, const size_t totalSize, const size_t offset, const bool instanced)
  {
    const uint32_t attributeIndex = shader_getAttributeIndex(pShader, vb_attribute_mat4<TAttributeName>::getAttributeName());

    for (uint32_t i = 0; i < 4; i++)
      glEnableVertexAttribArray(attributeIndex + i);

    for (uint32_t i = 0; i < 4; i++)
      glVertexAttribPointer(attributeIndex + i, (GLint)sizeof(float_t), vb_attribute_mat4<TAttributeName>::getDataType(), GL_FALSE, (GLsizei)totalSize, (const void *)(offset + sizeof(float_t) * 4 * i));

    if (instanced)
      for (uint32_t i = 0; i < 4; i++)
        glVertexAttribDivisor(attributeIndex + i, 1);
  }
};

template <typename T, typename... Args>
struct vb_attributeQuery_internal <T, Args...>
{
  static size_t getSize() { return T::getDataSize() + vb_attributeQuery_internal<Args...>::getSize(); };

  static void setAttribute(shader *pShader, const size_t totalSize, const size_t offset, const bool instanced)
  {
    vb_attributeQuery_internal<T>::setAttribute(pShader, totalSize, offset, instanced);
    vb_attributeQuery_internal<Args...>::setAttribute(pShader, totalSize, offset + T::getDataSize(), instanced);
  }
};

template <typename... Args>
void vb_attributeQuery_internal_setAttributes(shader *pShader, const bool instanced = false)
{
  const size_t totalSize = vb_attributeQuery_internal<Args...>::getSize();
  vb_attributeQuery_internal<Args...>::setAttribute(pShader, totalSize, 0, instanced);
}

//////////////////////////////////////////////////////////////////////////

// Template Parameters should be `vb_attribute`s like `vb_attribute_float`.
template <typename... Args>
struct vertexBuffer
{
  size_t count = 0;
  shader *pShader = nullptr;
  GLuint vao = 0;
  GLuint vbo = 0;
  GLenum renderMode = GL_TRIANGLES;
  bool initialized = false;
};

//////////////////////////////////////////////////////////////////////////

template<typename ...Args>
inline lsResult vertexBuffer_create(vertexBuffer<Args...> *pBuffer, shader *pShader)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr || pShader == nullptr, lsR_ArgumentNull);

  pBuffer->initialized = false; // Upload buffer data to validate buffer.
  pBuffer->pShader = pShader;
  
  glGenVertexArrays(1, &pBuffer->vao);
  glBindVertexArray(pBuffer->vao);

  glGenBuffers(1, &pBuffer->vbo);

epilogue:
  return result;
}

template<typename ...Args>
inline void vertexBuffer_destroy(vertexBuffer<Args...> *pBuffer)
{
  if (pBuffer->vao != 0)
  {
    glDeleteVertexArrays(1, &pBuffer->vao);
    pBuffer->vao = 0;
  }
  
  if (pBuffer->vbo != 0)
  {
    glDeleteBuffers(1, &pBuffer->vbo);
    pBuffer->vbo = 0;
  }

  pBuffer->pShader = nullptr;
}

template<typename U, typename ...Args>
inline lsResult vertexBuffer_setVertexBuffer(vertexBuffer<Args...> *pBuffer, const U *pData, const size_t count, const bool constantlyChangingVertices = false)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr || pData == nullptr, lsR_ArgumentNull);

  const size_t singleBlockSize = vb_attributeQuery_internal<Args...>::getSize();
  LS_ERROR_IF(singleBlockSize == 0, lsR_ResourceStateInvalid);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, pBuffer->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(U) * count, pData, constantlyChangingVertices ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  pBuffer->initialized = true;
  pBuffer->count = (sizeof(U) * count) / singleBlockSize;

epilogue:
  return result;
}

template<typename ...Args>
inline void vertexBuffer_setCount(vertexBuffer<Args...> *pBuffer, const size_t count)
{
  lsAssert(pBuffer != nullptr);
  lsAssert(pBuffer->initialized);

  pBuffer->count = count;
}

template<typename ...Args>
inline void vertexBuffer_setAttributes(vertexBuffer<Args...> *pBuffer)
{
  lsAssert(pBuffer != nullptr);
  lsAssert(pBuffer->initialized);

  glBindVertexArray(pBuffer->vao);
  glBindBuffer(GL_ARRAY_BUFFER, pBuffer->vbo);

  shader_bind(pBuffer->pShader);
  vb_attributeQuery_internal_setAttributes<Args...>(pBuffer->pShader);
}

template<typename ...Args>
inline void vertexBuffer_render(vertexBuffer<Args...> *pBuffer)
{
  vertexBuffer_setAttributes(pBuffer);

  glDrawArrays(pBuffer->renderMode, 0, (GLsizei)pBuffer->count);
}

// Template Parameter 1 should be a `vertexBuffer` of the model being instanced.
// Remaining Template Parameters should be `vb_attribute`s like `vb_attribute_float`.
template <typename T, typename... Args>
struct instancedVertexBuffer
{
  T instancedBuffer;
  size_t instanceCount = 0;
  GLuint instanceVBO = 0;
  bool hasInstanceData = false;
};

template<typename T, typename ...Args>
inline lsResult instancedVertexBuffer_create(instancedVertexBuffer<T, Args...> *pBuffer, shader *pShader)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr || pShader == nullptr, lsR_ArgumentNull);

  pBuffer->hasInstanceData = false;

  LS_ERROR_CHECK(vertexBuffer_create(&pBuffer->instancedBuffer, pShader));

  glGenBuffers(1, &pBuffer->instanceVBO);

epilogue:
  return result;
}

template<typename T, typename ...Args>
inline void instancedVertexBuffer_destroy(instancedVertexBuffer<T, Args...> *pBuffer)
{
  if (pBuffer->instanceVBO != 0)
  {
    glDeleteBuffers(1, &pBuffer->instanceVBO);
    pBuffer->instanceVBO = 0;
  }

  vertexBuffer_destroy(&pBuffer->instancedBuffer);
}

template<typename U, typename T, typename ...Args>
inline lsResult instancedVertexBuffer_setInstancedVertexBuffer(instancedVertexBuffer<T, Args...> *pBuffer, const U *pData, const size_t count, const bool constantlyChangingVertices = false)
{
  return vertexBuffer_setVertexBuffer(&pBuffer->instancedBuffer, pData, count, constantlyChangingVertices);
}

template<typename U, typename T, typename ...Args>
inline lsResult instancedVertexBuffer_setInstanceBuffer(instancedVertexBuffer<T, Args...> *pBuffer, const U *pData, const size_t count, const bool constantlyChangingData = false)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBuffer == nullptr || pData == nullptr, lsR_ArgumentNull);

  const size_t singleBlockSize = vb_attributeQuery_internal<Args...>::getSize();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, pBuffer->instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(U) * count, pData, constantlyChangingData ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  pBuffer->hasInstanceData = true;
  pBuffer->instanceCount = (sizeof(U) * count) / singleBlockSize;

epilogue:
  return result;
}

template<typename T, typename ...Args>
inline void instancedVertexBuffer_setInstancedVertexCount(instancedVertexBuffer<T, Args...> *pBuffer, const size_t count)
{
  vertexBuffer_setCount(&pBuffer->instancedBuffer, count);
}

template<typename T, typename ...Args>
inline void instancedVertexBuffer_setInstanceCount(instancedVertexBuffer<T, Args...> *pBuffer, const size_t count)
{
  pBuffer->instanceCount = count;
}

template<typename T, typename ...Args>
inline void instancedVertexBuffer_render(instancedVertexBuffer<T, Args...> *pBuffer)
{
  vertexBuffer_setAttributes(&pBuffer->instancedBuffer);

  lsAssert(pBuffer->instancedBuffer.initialized);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, pBuffer->instanceVBO);

  vb_attributeQuery_internal_setAttributes<Args...>(pBuffer->instancedBuffer.pShader, true);

  glDrawArraysInstanced(pBuffer->instancedBuffer.renderMode, 0, (GLsizei)pBuffer->instancedBuffer.count, (GLsizei)pBuffer->instanceCount);
}
