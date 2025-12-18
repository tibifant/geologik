#include "terrain.h"

lsResult terrain_init(_Out_ terrain *pTerrain, const uint16_t width, const uint16_t height)
{
  lsResult result = lsR_Success;

  pTerrain->width = width;
  pTerrain->height = height;

  LS_ERROR_CHECK(lsAllocZero(&(pTerrain->pTiles), width * height));

epilogue:
  return result;
}

void terrain_generate(terrain *pTerrain)
{
  lsAssert(pTerrain != nullptr);

  size_t i = 0;

  for (size_t y = 0; y < pTerrain->height; y++)
  {
    for (size_t x = 0; x < pTerrain->width; x++, i++)
    {
      const uint16_t height = (uint16_t)((lsSin(x * 0.1f) + lsCos(y * 0.1f)) * 100 + 255);

      for (size_t tt = 6; tt < tt_bedrock; tt++)
        pTerrain->pTiles[i].layerHeights[tt] = height;

      pTerrain->pTiles[i].layerHeights[tt_bedrock] = 8;
    }
  }
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

// next steps: fixed point, template for on bounds cases
template <typename T>
void generate_noise_recursive(T *pBuffer, const size_t filledWidth, const size_t maxWidth)
{
  // TODO variable influence factor

  const size_t filledCount = filledWidth * filledWidth;
  const size_t targetFilledCount = filledCount * 4;
  const size_t targetFilledWidth = filledWidth * 2;
  const size_t maxCount = maxWidth * maxWidth;

  lsAssert(targetFilledCount < maxCount);

  const size_t offset = maxCount - targetFilledCount;
  const size_t filledOffset = maxCount - filledCount;

  for (size_t y = 0; y < targetFilledWidth; y++)
  {
    for (size_t x = 0; x < targetFilledWidth; x++)
    {
      const size_t idx = y * targetFilledWidth + x + offset;

      // Random generation

      pBuffer[idx] = 0.3 * (T)(lsGetRand());

      // Upscaling

      constexpr float_t f = 0.7;

      if ((x == 0 && y == 0) || (x == targetFilledWidth - 1 && y == targetFilledWidth - 1))
      {
        // just self
        pBuffer[idx] += f * pBuffer[filledOffset];
        continue;
      }

      const int8_t xDir = (x % 2) ? 1 : -1;
      const int8_t yDir = (y % 2) ? 1 : -1;

      constexpr float_t threeQuarter = 0.75 * 0.25;
      constexpr float_t oneQuarter = 0.25 * 0.25;

      const vec2i parent = vec2u(x / 2, y / 2);
      const vec2i horizontal = parent + vec2i(xDir, 0);

      if (y == 0 || y == targetFilledWidth - 1)
      {
        // if y on bounds: 0.75 self + 0.25 horizontal
        pBuffer[idx] += f * T(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (horizontal.y * filledWidth + horizontal.x)]); // ?
        continue;
      }

      const vec2i vertical = parent + vec2i(0, yDir);
      if (x == 0 || x == targetFilledWidth - 1)
      {
        // if x on bounds: 0.75 self + 0.25 vertical
        pBuffer[idx] += f * T(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (vertical.y * filledWidth + vertical.x)]);
        continue;
      }

      T val = 0;

      val = T(2 * threeQuarter * pBuffer[filledOffset]); // 2*(0.75 / 4) parent
      val += T((threeQuarter + oneQuarter) * pBuffer[filledOffset + (horizontal.y * filledWidth + horizontal.x)]); // (0.75 / 4), (0.25 / 4) horizontal
      val += T((threeQuarter + oneQuarter) * pBuffer[filledOffset + (vertical.y * filledWidth + vertical.x)]); // (0.75 / 4), (0.25 / 4)

      const vec2i diagonal = parent + vec2i(xDir, yDir);
      val += T(2 * oneQuarter * pBuffer[filledOffset + (diagonal.y * filledWidth + diagonal.x)]); // 2 * (0.25 / 4)
      
      // todo if i just add them, i need to make sure there max size is not to big lol

      pBuffer[idx] += f * val;
    }
  }

  if (targetFilledWidth == maxWidth)
    return;

  generate_noise_revursive(pBuffer, targetFilledWidth, maxWidth);
}

void terrain_destroy(terrain *pTerrain)
{
  if (pTerrain == nullptr)
    return;

  lsFreePtr(&pTerrain->pTiles);
}

// TODO read/write data from/to file
