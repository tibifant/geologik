#include "texture.h"
#include "io.h"

#include "GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4100)
#include "stb_image.h"
#pragma warning(pop)

//////////////////////////////////////////////////////////////////////////

lsResult texture_create(texture *pTexture, const bool nearestNeighbor /* = false */)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pTexture == nullptr, lsR_ArgumentNull);

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &pTexture->textureId);

  glBindTexture(GL_TEXTURE_2D, pTexture->textureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearestNeighbor ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearestNeighbor ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  pTexture->initialized = true;

epilogue:
  return result;
}

lsResult texture_create(texture *pTexture, const char *filename, const bool nearestNeighbor /* = false */)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(texture_create(pTexture, nearestNeighbor));
  LS_ERROR_CHECK(texture_set(pTexture, filename));

epilogue:
  return result;
}

lsResult texture_set(texture *pTexture, const char *filename)
{
  lsResult result = lsR_Success;

  uint8_t *pFile = nullptr;
  stbi_uc *pImage = nullptr;

  LS_ERROR_IF(pTexture == nullptr || filename == nullptr, lsR_ArgumentNull);

  {
    size_t fileBytes = 0;
    LS_ERROR_CHECK(lsReadFile(filename, &pFile, &fileBytes));

    int32_t x, y, originalChannelCount;
    pImage = stbi_load_from_memory(pFile, (int32_t)fileBytes, &x, &y, &originalChannelCount, 4);

    LS_ERROR_IF(pImage == nullptr, lsR_InternalError);
    LS_ERROR_CHECK(texture_set(pTexture, pImage, vec2s(x, y)));
  }

epilogue:
  if (pFile != nullptr)
    lsFreePtr(&pFile);

  if (pImage != nullptr)
    stbi_image_free(pImage);

  return result;
}

lsResult texture_set_raw(texture *pTexture, const void *pData, texture_format_type textureFormatType, const vec2s resolution)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pTexture == nullptr || pData == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(!pTexture->initialized, lsR_ResourceStateInvalid);

  glBindTexture(GL_TEXTURE_2D, pTexture->textureId);

  switch (pTexture->textureFormatType = textureFormatType)
  {
  case tft_u8:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)resolution.x, (GLsizei)resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
    break;

  case tft_u16:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)resolution.x, (GLsizei)resolution.y, 0, GL_RGBA, GL_UNSIGNED_SHORT, pData);
    break;

  case tft_u8r:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)resolution.x, (GLsizei)resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
    break;

  default:
    lsAssert(false); // not implemented.
    break;
  }

  pTexture->resolution = resolution;
  pTexture->uploaded = true;

epilogue:
  return result;
}

lsResult texture_set(texture *pTexture, const uint8_t *pData, const vec2s resolution)
{
  return texture_set_raw(pTexture, pData, tft_u8, resolution);
}

lsResult texture_set(texture *pTexture, const uint16_t *pData, const vec2s resolution)
{
  return texture_set_raw(pTexture, pData, tft_u16, resolution);
}

lsResult texture_set_single(texture *pTexture, const uint8_t *pData, const vec2s resolution)
{
  return texture_set_raw(pTexture, pData, tft_u8r, resolution);
}

lsResult texture_bind(texture *pTexture, const uint32_t textureUnit)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pTexture == nullptr, lsR_InvalidParameter);
  LS_ERROR_IF(!pTexture->initialized, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pTexture->uploaded, lsR_ResourceStateInvalid);

  pTexture->textureUnit = textureUnit;

  glActiveTexture(GL_TEXTURE0 + textureUnit);
  glBindTexture(GL_TEXTURE_2D, pTexture->textureId);

epilogue:
  return result;
}

lsResult texture_bind_image(texture *pTexture, const uint32_t textureUnit)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pTexture == nullptr, lsR_InvalidParameter);
  LS_ERROR_IF(!pTexture->initialized, lsR_ResourceStateInvalid);
  LS_ERROR_IF(!pTexture->uploaded, lsR_ResourceStateInvalid);

  pTexture->textureUnit = textureUnit;

  switch (pTexture->textureFormatType)
  {
  case tft_u8:
    glBindImageTexture(textureUnit, pTexture->textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8); // only read write supported atm - other options available
    break;

  case tft_u16:
    glBindImageTexture(textureUnit, pTexture->textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16); // only read write supported atm - other options available
    break;

  case tft_u8r:
    glBindImageTexture(textureUnit, pTexture->textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8); // only read write supported atm - other options available
    break;

  default:
    lsAssert(false); // not implemented.
    break;
  }

epilogue:
  return result;
}

void texture_destroy(_Out_ texture *pTexture)
{
  if (pTexture == nullptr || !pTexture->initialized)
    return;

  glDeleteTextures(1, &pTexture->textureId);
  pTexture->textureId = 0;
}
