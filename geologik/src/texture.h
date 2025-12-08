#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

enum texture_format_type
{
  tft_unsigned_byte,
  tft_unsigned_short,
};

struct texture
{
  enum texture_format_type textureFormatType;
  vec2s resolution = vec2s(0);
  uint32_t textureId = 0; // this is actually a GLuint.
  uint32_t textureUnit = 0; // this is actually a GLuint.
  bool initialized = false;
  bool uploaded = false;
};

//////////////////////////////////////////////////////////////////////////

lsResult texture_create(_Out_ texture *pTexture, const bool nearestNeighbor = false);
lsResult texture_create(_Out_ texture *pTexture, const char *filename, const bool nearestNeighbor = false);
lsResult texture_set(texture *pTexture, const char *filename);
lsResult texture_set(texture *pTexture, const uint8_t *pData, const vec2s resolution);
lsResult texture_bind(texture *pTexture, const uint32_t textureUnit);
void texture_destroy(_Out_ texture *pTexture);
