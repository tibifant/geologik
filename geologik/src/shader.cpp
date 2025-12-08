#include "shader.h"

#include "texture.h"
#include "framebuffer.h"
#include "io.h"

#include <GL/glew.h>

//////////////////////////////////////////////////////////////////////////

thread_local static uint32_t shader_currentlyBoundIndex = (uint32_t)-1;

//////////////////////////////////////////////////////////////////////////

lsResult shader_create_vertex_fragment_internal(shader *pShader, const char *vertexSource, const char *fragmentSource, const bool requestNewProgram);
lsResult shader_create_compute_internal(shader *pShader, const char *computeSource, const bool requestNewProgram);
lsResult shader_allocCleanSource_internal(_In_ const char *source, _Out_ char **ppCleanSource);

//////////////////////////////////////////////////////////////////////////

lsResult shader_create_vertex_fragment(shader *pShader, const char *vertexSource, const char *fragmentSource)
{
  return shader_create_vertex_fragment_internal(pShader, vertexSource, fragmentSource, true);
}

lsResult shader_create_compute(shader *pShader, const char *computeSource)
{
  return shader_create_compute_internal(pShader, computeSource, true); // true? do we even need this i don't understand
}

lsResult shader_createFromFile_vertex_fragment(shader *pShader, const char *vertexPath, const char *fragmentPath)
{
  lsResult result = lsR_Success;

  char *vertexSource = nullptr;
  char *fragmentSource = nullptr;

  LS_ERROR_IF(pShader == nullptr || vertexPath == nullptr || fragmentPath == nullptr, lsR_ArgumentNull);

  size_t bytes = 0; // unused.
  LS_ERROR_CHECK(lsReadFile(vertexPath, &vertexSource, &bytes));
  LS_ERROR_CHECK(lsReadFile(fragmentPath, &fragmentSource, &bytes));

  LS_ERROR_CHECK(shader_create_vertex_fragment(pShader, vertexSource, fragmentSource));

//#ifdef _DEBUG
//  pShader->vertexPath = _strdup(vertexPath);
//  pShader->fragmentPath = _strdup(fragmentPath);
//#endif

epilogue:
  lsFreePtr(&vertexSource);
  lsFreePtr(&fragmentSource);

  return result;
}

lsResult shader_createFromFile_compute(shader *pShader, const char *computePath)
{
  lsResult result = lsR_Success;

  char *computeSource = nullptr;

  LS_ERROR_IF(pShader == nullptr || computePath == nullptr, lsR_ArgumentNull);

  size_t bytes = 0; // unused.
  LS_ERROR_CHECK(lsReadFile(computePath, &computeSource, &bytes));

  LS_ERROR_CHECK(shader_create_compute(pShader, computeSource));

epilogue:
  lsFreePtr(&computeSource);

  return result;
}

void shader_bind(shader *pShader)
{
  lsAssert(pShader != nullptr);
  lsAssert(pShader->initialized);

  if (shader_currentlyBoundIndex != pShader->shaderProgram)
    glUseProgram(pShader->shaderProgram);

  shader_currentlyBoundIndex = pShader->shaderProgram;
}

//#ifdef _DEBUG
//lsResult shader_reload(shader *pShader)
//{
//  lsResult result = lsR_Success;
//
//  char *vertexSource = nullptr;
//  char *fragmentSource = nullptr;
//
//  LS_ERROR_IF(pShader == nullptr, lsR_ArgumentNull);
//  LS_ERROR_IF(!pShader->initialized, lsR_ResourceStateInvalid);
//  LS_ERROR_IF(pShader->fragmentPath == nullptr || pShader->vertexPath == nullptr, lsR_ResourceStateInvalid);
//
//  size_t bytes = 0; // unused.
//  LS_ERROR_CHECK(lsReadFile(pShader->vertexPath, &vertexSource, &bytes));
//  LS_ERROR_CHECK(lsReadFile(pShader->fragmentPath, &fragmentSource, &bytes));
//
//  LS_ERROR_CHECK(shader_create_vertex_fragment(pShader, vertexSource, fragmentSource));
//
//epilogue:
//  lsFreePtr(&vertexSource);
//  lsFreePtr(&fragmentSource);
//
//  return result;
//}
//#endif

void shader_destroy(shader *pShader)
{
  if (!pShader->initialized)
    return;

  if (pShader->shaderProgram != 0)
  {
    glDeleteProgram(pShader->shaderProgram);
    pShader->shaderProgram = 0;
  }

//#ifdef _DEBUG
//  lsFreePtr(&pShader->vertexPath);
//  lsFreePtr(&pShader->fragmentPath);
//#endif

  lsFreePtr(&pShader->pUniformReferences);
  lsFreePtr(&pShader->pAttributeReferences);
}

//////////////////////////////////////////////////////////////////////////

uint32_t shader_getUniformIndex(shader *pShader, const char *uniformName)
{
  for (size_t i = 0; i < pShader->uniformReferenceCount; i++)
    if (strncmp(pShader->pUniformReferences[i].name, uniformName, LS_ARRAYSIZE(pShader->pUniformReferences[i].name)) == 0)
      return pShader->pUniformReferences[i].index;

  shader_name_ref ref;
  ref.index= glGetUniformLocation(pShader->shaderProgram, uniformName);
  lsAssert(ref.index != (uint32_t)-1);

  const size_t length = strlen(uniformName);

  if (length >= LS_ARRAYSIZE_C_STYLE(shader_name_ref::name))
    return ref.index;

  if (LS_FAILED(lsRealloc(&pShader->pUniformReferences, pShader->uniformReferenceCount + 1)))
    return ref.index;

  memcpy(ref.name, uniformName, length + 1);

  pShader->pUniformReferences[pShader->uniformReferenceCount] = ref;
  pShader->uniformReferenceCount++;

  return ref.index;
}

uint32_t shader_getAttributeIndex(shader *pShader, const char *attributeName)
{
  for (size_t i = 0; i < pShader->attributeReferenceCount; i++)
    if (strncmp(pShader->pAttributeReferences[i].name, attributeName, LS_ARRAYSIZE(pShader->pAttributeReferences[i].name)) == 0)
      return pShader->pAttributeReferences[i].index;

  shader_name_ref ref;
  ref.index = glGetAttribLocation(pShader->shaderProgram, attributeName);
  lsAssert(ref.index != (uint32_t)-1);
  
  const size_t length = strlen(attributeName);
  
  if (length >= LS_ARRAYSIZE_C_STYLE(shader_name_ref::name))
    return ref.index;

  if (LS_FAILED(lsRealloc(&pShader->pAttributeReferences, pShader->attributeReferenceCount + 1)))
    return ref.index;

  memcpy(ref.name, attributeName, length + 1);

  pShader->pAttributeReferences[pShader->attributeReferenceCount] = ref;
  pShader->attributeReferenceCount++;

  return ref.index;
}

//////////////////////////////////////////////////////////////////////////

void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const int32_t v) { shader_bind(pShader); glUniform1i(index, v); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const uint32_t v) { shader_bind(pShader); glUniform1ui(index, v); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const float_t v) { shader_bind(pShader); glUniform1f(index, v); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2f &v) { shader_bind(pShader); glUniform2f(index, v.x, v.y); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2i32 &v) { shader_bind(pShader); glUniform2i(index, v.x, v.y); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3f &v) { shader_bind(pShader); glUniform3f(index, v.x, v.y, v.z); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3i32 &v) { shader_bind(pShader); glUniform3i(index, v.x, v.y, v.z); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4f &v) { shader_bind(pShader); glUniform4f(index, v.x, v.y, v.z, v.w); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4i32 &v) { shader_bind(pShader); glUniform4i(index, v.x, v.y, v.z, v.w); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec &v) { shader_bind(pShader); glUniform4f(index, v.x, v.y, v.z, v.w); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const matrix &v) { shader_bind(pShader); glUniformMatrix4fv(index, 1, false, &v._11); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const texture *pV) { shader_bind(pShader); glUniform1i(index, (GLint)pV->textureUnit); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const framebuffer *pV) { shader_bind(pShader); glUniform1i(index, (GLint)pV->textureUnit); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const int32_t *pV, const size_t count) { shader_bind(pShader); glUniform1iv(index, (GLsizei)count, pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const uint32_t *pV, const size_t count) { shader_bind(pShader); glUniform1uiv(index, (GLsizei)count, pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const float_t *pV, const size_t count) { shader_bind(pShader); glUniform1fv(index, (GLsizei)count, pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec2f *pV, const size_t count) { shader_bind(pShader); glUniform2fv(index, (GLsizei)count, (float_t *)pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec3f *pV, const size_t count) { shader_bind(pShader); glUniform3fv(index, (GLsizei)count, (float_t *)pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec4f *pV, const size_t count) { shader_bind(pShader); glUniform4fv(index, (GLsizei)count, (float_t *)pV); }
void shader_setUniformAtIndex(shader *pShader, const uint32_t index, const vec *pV, const size_t count) { shader_bind(pShader); glUniform4fv(index, (GLsizei)count, (float_t *)pV); }

void shader_setUniformDepthStencilAtIndex(shader *pShader, const uint32_t index, const framebuffer *pV) { shader_bind(pShader); glUniform1i(index, (GLint)pV->textureUnitDepthStencil); }

//////////////////////////////////////////////////////////////////////////

lsResult shader_create_vertex_fragment_internal(shader *pShader, const char *vertexSource, const char *fragmentSource, const bool requestNewProgram)
{
  lsResult result = lsR_Success;

  char *cleanVertexSource = nullptr;
  char *cleanFragmentSource = nullptr;

  GLuint vertexShaderHandle = (GLuint)-1;
  GLuint fragmentShaderHandle = (GLuint)-1;

  LS_ERROR_IF(pShader == nullptr || vertexSource == nullptr || fragmentSource == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(shader_allocCleanSource_internal(vertexSource, &cleanVertexSource));
  LS_ERROR_CHECK(shader_allocCleanSource_internal(fragmentSource, &cleanFragmentSource));

  vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShaderHandle, 1, &vertexSource, NULL);
  glCompileShader(vertexShaderHandle);

  GLint status;
  glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[1024];
    glGetShaderInfoLog(vertexShaderHandle, sizeof(buffer), nullptr, buffer);

    puts("Error compiling vertex shader.\nThe following error occured:");
    puts(buffer);

    LS_ERROR_SET(lsR_ResourceInvalid);
  }

  fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShaderHandle);

  status = GL_TRUE;
  glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[1024];
    glGetShaderInfoLog(fragmentShaderHandle, sizeof(buffer), nullptr, buffer);

    puts("Error compiling fragment shader.\nThe following error occured:");
    puts(buffer);
    
    LS_ERROR_SET(lsR_ResourceInvalid);
  }

  if (requestNewProgram)
    pShader->shaderProgram = glCreateProgram();
  else
    glUseProgram(pShader->shaderProgram);

  glAttachShader(pShader->shaderProgram, vertexShaderHandle);
  glAttachShader(pShader->shaderProgram, fragmentShaderHandle);

  glLinkProgram(pShader->shaderProgram);

  glGetProgramiv(pShader->shaderProgram, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[1024];
    glGetProgramInfoLog(pShader->shaderProgram, sizeof(buffer), nullptr, buffer);

    puts("Error linking shader.\nThe following error occured:");
    puts(buffer);

    LS_ERROR_SET(lsR_ResourceInvalid);
  }

  pShader->initialized = true;

epilogue:
  lsFreePtr(&cleanVertexSource);
  lsFreePtr(&cleanFragmentSource);
  
  if (vertexShaderHandle != (GLuint)-1)
    glDeleteShader(vertexShaderHandle);

  if (fragmentShaderHandle != (GLuint)-1)
    glDeleteShader(fragmentShaderHandle);

  if (LS_FAILED(result) && pShader->shaderProgram != 0 && requestNewProgram)
  {
    glDeleteProgram(pShader->shaderProgram);
    pShader->shaderProgram = 0;
  }

  return result;
}

lsResult shader_create_compute_internal(shader *pShader, const char *computeSource, const bool requestNewProgram)
{
  lsResult result = lsR_Success;

  char *cleanSource = nullptr;

  GLuint shaderHandle = (GLuint)-1;

  LS_ERROR_IF(pShader == nullptr || computeSource == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(shader_allocCleanSource_internal(computeSource, &cleanSource));

  shaderHandle = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shaderHandle, 1, &computeSource, NULL);
  glCompileShader(shaderHandle);

  GLint status;
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status); //?

  if (status != GL_TRUE)
  {
    char buffer[1024];
    glGetShaderInfoLog(shaderHandle, sizeof(buffer), nullptr, buffer);

    puts("Error compiling compute shader.\nThe following error occured:");
    puts(buffer);

    LS_ERROR_SET(lsR_ResourceInvalid);
  }

  if (requestNewProgram)
    pShader->shaderProgram = glCreateProgram();
  else
    glUseProgram(pShader->shaderProgram);

  glAttachShader(pShader->shaderProgram, shaderHandle);
  
  glLinkProgram(pShader->shaderProgram);

  glGetProgramiv(pShader->shaderProgram, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[1024];
    glGetProgramInfoLog(pShader->shaderProgram, sizeof(buffer), nullptr, buffer);

    puts("Error linking shader.\nThe following error occured:");
    puts(buffer);

    LS_ERROR_SET(lsR_ResourceInvalid);
  }

  pShader->initialized = true;

epilogue:
  lsFreePtr(&cleanSource);

  if (shaderHandle != (GLuint)-1)
    glDeleteShader(shaderHandle);

  if (LS_FAILED(result) && pShader->shaderProgram != 0 && requestNewProgram)
  {
    glDeleteProgram(pShader->shaderProgram);
    pShader->shaderProgram = 0;
  }

  return result;
}

lsResult shader_allocCleanSource_internal(_In_ const char *source, _Out_ char **ppCleanSource)
{
  lsResult result = lsR_Success;

  char *cleanSource = nullptr;
  const size_t length = strlen(source) + 1;

  LS_ERROR_CHECK(lsAlloc(&cleanSource, length));

  char *write = cleanSource;

  for (size_t i = 0; i < length; i++)
  {
    if (source[i] != '\r')
    {
      *write = source[i];
      write++;
    }
  }

  *ppCleanSource = cleanSource;

epilogue:
  if (LS_FAILED(result))
    lsFreePtr(&cleanSource);

  return result;
}
