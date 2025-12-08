#pragma once

#include "core.h"

struct framebuffer
{
  vec2s size;
  uint32_t textureUnit, textureUnitDepthStencil;
  uint32_t frameBufferHandle; // actually a GLuint.
  uint32_t textureId; // actually a GLuint.
  uint32_t depthStencilTextureId; // actually a GLuint.
  size_t sampleCount;
  bool hasDepthStencil;
};

lsResult framebuffer_create(framebuffer *pFramebuffer, const vec2s size, const size_t sampleCount, const bool hasDepthStencil);
void framebuffer_destroy(framebuffer *pFramebuffer);

void framebuffer_bind(framebuffer *pFramebuffer);
void framebuffer_unbind();

void framebuffer_setResolution(framebuffer *pFramebuffer, const vec2s size);
void framebuffer_blitToScreen(framebuffer *pFramebuffer, const vec2s renderResolution);
void framebuffer_blitToFramebuffer(framebuffer *pSource, framebuffer *pTarget);

lsResult texture_bind(framebuffer *pFramebuffer, const uint32_t textureUnit);
lsResult texture_bindDepthStencil(framebuffer *pFramebuffer, const uint32_t textureUnit);
