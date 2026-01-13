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
void generate_noise_recursive(T *pBuffer, const size_t filledWidth, const size_t maxWidth, const float_t scaleGradient, const float_t scaleFactor)
{
  // TODO variable influence factor

  const size_t filledCount = filledWidth * filledWidth;
  const size_t targetFilledWidth = filledWidth * 2;
  const size_t targetFilledCount = targetFilledWidth * targetFilledWidth;
  const size_t maxCount = maxWidth * maxWidth;

  lsAssert(targetFilledCount <= maxCount);

  const size_t offset = maxCount - targetFilledCount;
  const size_t filledOffset = maxCount - filledCount;
  const T *pReadBuffer = pBuffer + filledOffset;
  T *pWriteBuffer = pBuffer + offset;

  const float_t scaledInfluence = scaleFactor;
  const float_t noiseInfluence = 1 - scaledInfluence;

  for (size_t y = 0; y < targetFilledWidth; y++)
  {
    for (size_t x = 0; x < targetFilledWidth; x++)
    {
      const size_t idx = y * targetFilledWidth + x;

      // Upscaling

      const int8_t xDir = (x & 1) ? 1 : -1;
      const int64_t yDir = (y & 1) ? filledWidth : -(int64_t)filledWidth;

      const vec2i parent = vec2i(x / 2, y / 2);
      const size_t parentIdx = parent.y * filledWidth + parent.x;
      const size_t horizontalParentIdx = parentIdx + xDir;
      const size_t verticalParentIdx = parentIdx + yDir;
      const size_t diagonalParentIdx = parentIdx + xDir + yDir;

      uint64_t scaled;

      if ((x == 0 || x == targetFilledWidth - 1) && (y == 0 || y == targetFilledWidth - 1))
      {
        // just self
        scaled = pReadBuffer[parentIdx];
      }
      else if (y == 0 || y == targetFilledWidth - 1)
      {
        // if y on bounds: 0.75 self + 0.25 horizontal
        scaled = (uint64_t)lsRound(0.75 * pReadBuffer[parentIdx] + 0.25 * pReadBuffer[horizontalParentIdx]);
      }
      else if (x == 0 || x == targetFilledWidth - 1)
      {
        // if y on bounds: 0.75 self + 0.25 horizontal
        scaled = (uint64_t)lsRound(0.75 * pReadBuffer[parentIdx] + 0.25 * pReadBuffer[verticalParentIdx]);;
      }
      else
      {
        const double top = 0.75 * pReadBuffer[parentIdx] + 0.25 * pReadBuffer[horizontalParentIdx];
        const double bottom = 0.75 * pReadBuffer[verticalParentIdx] + 0.25 * pReadBuffer[diagonalParentIdx];
        scaled = (uint64_t)lsRound(0.75 * top + 0.25 * bottom);
      }

      // Random generation
      const uint64_t noise = (T)lsGetRand();
      const uint64_t val = (uint64_t)lsRound(scaledInfluence * scaled + noiseInfluence * noise);

      lsAssert(val <= lsMaxValue<T>());
      pWriteBuffer[idx] = T(val);
    }
  }

  if (targetFilledWidth == maxWidth)
    return;

  print("Noise Scaling: ", targetFilledWidth, " done! (", scaledInfluence, " vs ", noiseInfluence, ")\n");

  FILE *pFile = fopen(sformat("C:\\data\\noise", targetFilledWidth, ".raw"), "wb");
  fwrite(pWriteBuffer, sizeof(T), targetFilledCount, pFile);
  fflush(pFile);
  fclose(pFile);

  generate_noise_recursive(pBuffer, targetFilledWidth, maxWidth, scaleGradient, scaleFactor + (1 - scaleFactor) * scaleGradient);
}

template <typename T>
void generate_noise(T *pBuffer, const size_t targetWidth)
{
  lsAssert(pBuffer != nullptr);
  lsAssert(__popcnt64(targetWidth) == 1); // must be a power of 2
  lsAssert(targetWidth >= 4);

  const size_t targetCount = targetWidth * targetWidth;

  pBuffer[targetCount - 1] = T(lsGetRand());
  pBuffer[targetCount - 2] = T(lsGetRand());
  pBuffer[targetCount - 3] = T(lsGetRand());
  pBuffer[targetCount - 4] = T(lsGetRand());

  generate_noise_recursive(pBuffer, 2, targetWidth, 0.5f, 0.5f);
}

