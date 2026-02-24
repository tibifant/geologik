#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

enum texture_format_type
{
  tft_unsigned_invalid,
  tft_u8,
  tft_u16,
  tft_f32,
  tft_u8r,
};

enum texture_image_access
{
  tia_read_only,
  tia_write_only,
  tia_read_write,
};

struct texture
{
  texture_format_type textureFormatType = tft_unsigned_invalid;
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
lsResult texture_set_raw(texture *pTexture, const void *pData, texture_format_type textureFormatType, const vec2s resolution);
lsResult texture_set(texture *pTexture, const uint8_t *pData, const vec2s resolution);
lsResult texture_set(texture *pTexture, const uint16_t *pData, const vec2s resolution);
lsResult texture_set_u8r(texture *pTexture, const uint8_t *pData, const vec2s resolution);
lsResult texture_bind(texture *pTexture, const uint32_t textureUnit);
lsResult texture_bind_image(texture *pTexture, const uint32_t textureUnit, const texture_image_access accessType);
void texture_destroy(_Out_ texture *pTexture);
