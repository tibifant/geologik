#pragma once

#include "platform.h"

struct shader_name_ref
{
  char name[28];
  uint32_t index; // actually a GLuint.
};

enum shader_type
{
  st_vertex_fragment,
  st_compute,
};

struct shader
{
  uint32_t shaderProgram = 0; // actually a GLuint.
  bool initialized = false;
  
  enum shader_type type;

//#ifdef _DEBUG
//  // if loaded from file:
//  const char *vertexPath = nullptr;
//  const char *fragmentPath = nullptr;
//#endif

  shader_name_ref *pUniformReferences = nullptr;
  size_t uniformReferenceCount = 0;
  shader_name_ref *pAttributeReferences = nullptr;
  size_t attributeReferenceCount = 0;
};

//////////////////////////////////////////////////////////////////////////

lsResult shader_create_vertex_fragment(_Out_ shader *pShader, const char *vertexSource, const char *fragmentSource);
lsResult shader_create_compute(_Out_ shader *pShader, const char *computeSource);
lsResult shader_createFromFile_vertex_fragment(_Out_ shader *pShader, const char *vertexPath, const char *fragmentPath);
lsResult shader_createFromFile_compute(shader *pShader, const char *computePath);

void shader_bind(shader *pShader);

//#ifdef _DEBUG
//lsResult shader_reload(shader *pShader);
//#endif

void shader_destroy(shader *pShader);

//////////////////////////////////////////////////////////////////////////

uint32_t shader_getUniformIndex(shader *pShader, const char *uniformName);
uint32_t shader_getAttributeIndex(shader *pShader, const char *attributeName);

//////////////////////////////////////////////////////////////////////////

void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const int32_t v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const uint32_t v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const float_t v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2f &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2i32 &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3f &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3i32 &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4f &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4i32 &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const matrix &v);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const struct texture *pV);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const struct framebuffer *pV);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const int32_t *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const uint32_t *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const float_t *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2f *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3f *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4f *pV, const size_t count);
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec *pV, const size_t count);

void shader_setUniformDepthStencilAtIndex(shader *pShader, const uint32_t index, const struct framebuffer *pV);

//////////////////////////////////////////////////////////////////////////

template <typename T>
void shader_setUniform(shader *pShader, const char *uniformName, T v)
{
  shader_setUniformAtIndex(pShader, shader_getUniformIndex(pShader, uniformName), v);
}

template <typename T, size_t TCount>
void shader_setUniform(shader *pShader, const char *uniformName, T v[TCount])
{
  void shader_setUniformAtIndex(pShader, shader_getUniformIndex(pShader, uniformName), v, TCount);
}

template <typename T>
void shader_setUniform(shader *pShader, const char *uniformName, T *pV, const size_t count)
{
  void shader_setUniformAtIndex(pShader, shader_getUniformIndex(pShader, uniformName), pV, count);
}

template <typename T>
void shader_setUniformDepthStencil(shader *pShader, const char *uniformName, T v)
{
  shader_setUniformDepthStencilAtIndex(pShader, shader_getUniformIndex(pShader, uniformName), v);
}
