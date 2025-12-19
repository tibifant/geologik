#pragma once

#include "core.h"

constexpr uint8_t _Version = 1;

enum terrain_type
{
  tt_snow,
  tt_water,
  tt_grass,
  tt_soil,
  tt_sand,
  tt_limestone,
  tt_stone,
  tt_bedrock,

  tt_count
};

struct tile
{
  uint16_t layerHeights[tt_count]; // in decimeters
};

struct terrain
{
  uint16_t width;

  tile *pTiles = nullptr;
};

lsResult terrain_init(_Out_ terrain *pTerrain, const uint16_t width);
void terrain_generate(terrain *pTerrain);
void terrain_destroy(terrain *pTerrain);

//////////////////////////////////////////////////////////////////////////

// next steps: fixed point, template for on bounds cases
template <typename T>
void generate_noise_recursive(T *pBuffer, const size_t filledWidth, const size_t maxWidth)
{
  // TODO variable influence factor

  const size_t filledCount = filledWidth * filledWidth;
  const size_t targetFilledWidth = filledWidth * 2;
  const size_t targetFilledCount = targetFilledWidth * targetFilledWidth;
  const size_t maxCount = maxWidth * maxWidth;

  lsAssert(targetFilledCount <= maxCount);

  const size_t offset = maxCount - targetFilledCount;
  const size_t filledOffset = maxCount - filledCount;

  FILE *pFile = fopen(sformat("C:\\data\\noise", filledWidth, ".raw"), "wb");
  fwrite(pBuffer + filledOffset, sizeof(uint16_t), filledCount, pFile);
  fflush(pFile);
  fclose(pFile);

  for (size_t y = 0; y < targetFilledWidth; y++)
  {
    for (size_t x = 0; x < targetFilledWidth; x++)
    {
      const size_t idx = y * targetFilledWidth + x + offset;

      // Random generation

      //pBuffer[idx] = 0.1 * (T)(lsGetRand());

      // Upscaling

      constexpr float_t f = 1;//0.9;

      if ((x == 0 && y == 0) || (x == targetFilledWidth - 1 && y == targetFilledWidth - 1))
      {
        // just self
        const uint64_t val = f * pBuffer[filledOffset];
        lsAssert(val <= lsMaxValue<uint16_t>());

        pBuffer[idx] = val;
        continue;
      }

      const int8_t xDir = (x % 2) ? 1 : -1;
      const int8_t yDir = (y % 2) ? 1 : -1;

      constexpr float_t threeQuarter = 0.75 * 0.25;
      constexpr float_t oneQuarter = 0.25 * 0.25;

      const vec2i parent = vec2i(x / 2, y / 2);
      const vec2i horizontal = parent + vec2i(xDir, 0);

      if (y == 0 || y == targetFilledWidth - 1)
      {
        // if y on bounds: 0.75 self + 0.25 horizontal
        const uint64_t val = f * uint64_t(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (horizontal.y * filledWidth + horizontal.x)]); // ?
        lsAssert(val <= lsMaxValue<uint16_t>());
        pBuffer[idx] = val;
        continue;
      }

      const vec2i vertical = parent + vec2i(0, yDir);
      if (x == 0 || x == targetFilledWidth - 1)
      {
        // if x on bounds: 0.75 self + 0.25 vertical
        const uint64_t val = f * uint64_t(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (vertical.y * filledWidth + vertical.x)]);
        lsAssert(val <= lsMaxValue<uint16_t>());
        pBuffer[idx] = val;
        continue;
      }

      uint64_t val = 0;

      val = uint64_t(2 * threeQuarter * pBuffer[filledOffset]); // 2*(0.75 / 4) parent
      val += uint64_t((threeQuarter + oneQuarter) * pBuffer[filledOffset + (horizontal.y * filledWidth + horizontal.x)]); // (0.75 / 4), (0.25 / 4) horizontal
      val += uint64_t((threeQuarter + oneQuarter) * pBuffer[filledOffset + (vertical.y * filledWidth + vertical.x)]); // (0.75 / 4), (0.25 / 4) vertical

      const vec2i diagonal = parent + vec2i(xDir, yDir);
      val += uint64_t(2 * oneQuarter * pBuffer[filledOffset + (diagonal.y * filledWidth + diagonal.x)]); // 2 * (0.25 / 4) diagonal

      lsAssert(val <= lsMaxValue<uint16_t>());

      // todo if i just add them, i need to make sure there max size is not to big lol

      pBuffer[idx] += T(f * val);
    }
  }

  if (targetFilledWidth == maxWidth)
    return;

  generate_noise_recursive(pBuffer, targetFilledWidth, maxWidth);
}

template <typename T>
void generate_noise(T *pBuffer, const size_t targetWidth)
{
  lsAssert(pBuffer != nullptr);
  lsAssert(__popcnt(targetWidth) == 1); // must be a power of 2

  const size_t targetCount = targetWidth * targetWidth;

  pBuffer[targetCount - 1] = T(lsGetRand());
  pBuffer[targetCount - 2] = T(lsGetRand());
  pBuffer[targetCount - 3] = T(lsGetRand());
  pBuffer[targetCount - 4] = T(lsGetRand());

  generate_noise_recursive(pBuffer, 2, targetWidth);
}

