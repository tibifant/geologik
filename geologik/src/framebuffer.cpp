#include "framebuffer.h"

#include <GL/glew.h>

lsResult framebuffer_create(framebuffer *pFramebuffer, const vec2s size, const size_t sampleCount, const bool hasDepthStencil)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pFramebuffer == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(size.x > UINT32_MAX || size.y > UINT32_MAX, lsR_ArgumentOutOfBounds);

  pFramebuffer->size = size;
  pFramebuffer->textureUnit = (uint32_t)-1;
  pFramebuffer->textureUnitDepthStencil = (uint32_t)-1;
  pFramebuffer->sampleCount = sampleCount;
  pFramebuffer->hasDepthStencil = hasDepthStencil;

  GLenum glPixelFormat = GL_RGBA;
  GLenum glType = GL_UNSIGNED_BYTE;

  glGenFramebuffers(1, &pFramebuffer->frameBufferHandle);
  glBindFramebuffer(GL_FRAMEBUFFER, pFramebuffer->frameBufferHandle);

  glGenTextures(1, &pFramebuffer->textureId);
  glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->textureId);

  if (pFramebuffer->sampleCount > 0)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)pFramebuffer->sampleCount, glPixelFormat, (GLsizei)size.x, (GLsizei)size.y, true);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, glPixelFormat, (GLsizei)size.x, (GLsizei)size.y, 0, glPixelFormat, glType, nullptr);

  if (pFramebuffer->sampleCount == 0)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->textureId, 0);

  if (pFramebuffer->hasDepthStencil)
  {
    glGenTextures(1, &pFramebuffer->depthStencilTextureId);
    glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->depthStencilTextureId);

    if (pFramebuffer->sampleCount == 0)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    if (pFramebuffer->sampleCount > 0)
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)pFramebuffer->sampleCount, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y, true);
    else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, (pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->depthStencilTextureId, 0);
  }

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  LS_ERROR_IF(status != GL_FRAMEBUFFER_COMPLETE, lsR_InternalError);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

epilogue:
  return result;
}

void framebuffer_destroy(framebuffer *pFramebuffer)
{
  framebuffer_unbind();

  if (pFramebuffer->frameBufferHandle)
    glDeleteFramebuffers(1, &pFramebuffer->frameBufferHandle);

  pFramebuffer->frameBufferHandle = 0;

  if (pFramebuffer->textureId)
    glDeleteTextures(0, &pFramebuffer->textureId);

  pFramebuffer->textureId = 0;
}

void framebuffer_bind(framebuffer *pFramebuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, pFramebuffer->frameBufferHandle);
}

void framebuffer_unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_setResolution(framebuffer *pFramebuffer, const vec2s size)
{
  pFramebuffer->size = size;

  glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->textureId);

  if (pFramebuffer->sampleCount > 0)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)pFramebuffer->sampleCount, GL_RGBA, (GLsizei)size.x, (GLsizei)size.y, true);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  if (pFramebuffer->sampleCount == 0)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  if (pFramebuffer->hasDepthStencil)
  {
    glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->depthStencilTextureId);

    if (pFramebuffer->sampleCount > 0)
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)pFramebuffer->sampleCount, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y, true);
    else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, nullptr);

    if (pFramebuffer->sampleCount == 0)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
  }
}

void framebuffer_blitToScreen(framebuffer *pFramebuffer, const vec2s renderResolution)
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, pFramebuffer->frameBufferHandle);

  glDrawBuffer(GL_BACK);

  glBlitFramebuffer(0, 0, (GLsizei)pFramebuffer->size.x, (GLsizei)pFramebuffer->size.y, 0, 0, (GLsizei)renderResolution.x, (GLsizei)renderResolution.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void framebuffer_blitToFramebuffer(framebuffer *pSource, framebuffer *pTarget)
{
  framebuffer_bind(pTarget);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pTarget->frameBufferHandle);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, pSource->frameBufferHandle);

  glBlitFramebuffer(0, 0, (GLsizei)pSource->size.x, (GLsizei)pSource->size.y, 0, 0, (GLsizei)pTarget->size.x, (GLsizei)pTarget->size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  framebuffer_unbind();
}

lsResult texture_bind(framebuffer *pFramebuffer, const uint32_t textureUnit)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pFramebuffer == nullptr, lsR_InvalidParameter);

  pFramebuffer->textureUnit = textureUnit;

  glActiveTexture(GL_TEXTURE0 + textureUnit);
  glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->textureId);

epilogue:
  return result;
}

lsResult texture_bindDepthStencil(framebuffer *pFramebuffer, const uint32_t textureUnit)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pFramebuffer == nullptr, lsR_InvalidParameter);
  LS_ERROR_IF(!pFramebuffer->hasDepthStencil, lsR_ResourceInvalid);

  pFramebuffer->textureUnitDepthStencil = textureUnit;

  glActiveTexture(GL_TEXTURE0 + textureUnit);
  glBindTexture((pFramebuffer->sampleCount > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, pFramebuffer->depthStencilTextureId);

epilogue:
  return result;
}
