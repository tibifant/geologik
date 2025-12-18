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
void generate_noise(T *pBuffer, const size_t targetCount)
{
  LS_ERROR_IF(pBuffer == nullptr, lsR_ArgumentNull);

  pBuffer[targetCount - 1] = T(lsGetRand());
  pBuffer[targetCount - 2] = T(lsGetRand());
  pBuffer[targetCount - 3] = T(lsGetRand());
  pBuffer[targetCount - 4] = T(lsGetRand());

  generate_noise_recursive(pBuffer, 4, targetCount);
}

// next steps: fixed point, template for on bounds cases
template <typename T>
void generate_noise_recursive(T *pBuffer, const size_t filledCount, const size_t maxCount)
{
  // TODO influence factor to prefer starting values

  const size_t offset = maxCount - filledCount * filledCount;
  const size_t filledOffset = maxCount - filledCount;

  for (size_t y = 0; y < filledCount; y++)
  {
    for (size_t x = 0; x < filledCount; x++)
    {
      const size_t idx = y * filledCount + x + offset;

      // todo: handle edge tiles!
      if ((x == 0 && y == 0) || (x == filledCount - 1 && y == filledCount - 1))
      {
        // just self
        pBuffer[idx] = pBuffer[filledOffset];
        continue;
      }

      const int8_t xDir = (x % 2) ? 1 : -1;
      const int8_t yDir = (y % 2) ? 1 : -1;

      constexpr float_t threeQuarter = 0.75 * 0.25;
      constexpr float_t oneQuarter = 0.25 * 0.25;

      const vec2i parent = vec2u(x / 2, y / 2);
      const vec2i horizontal = parent + vec2i(xDir, 0);

      if (y == 0 || y == filledCount - 1)
      {
        // if y on bounds: 0.75 self + 0.25 horizontal
        pBuffer[idx] = T(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (horizontal.y * filledCount / 2 + horizontal.x]));
        continue;
      }

      const vec2i vertical = parent + vec2i(0, yDir);
      if (x == 0 || x == filledCount - 1)
      {
        // if x on bounds: 0.75 self + 0.25 vertical
        pBuffer[idx] = T(0.75 * pBuffer[filledOffset] + 0.25 * pBuffer[filledOffset + (vertical.y * filledCount / 2 + vertical.x]));
        continue;
      }

      pBuffer[idx] = T(2 * threeQuarter * pBuffer[filledOffset]); // 2*(0.75 / 4) parent
      pBuffer[idx] += T((threeQuarter + oneQuarter) * pBuffer[filledOffset + (horizontal.y * filledCount / 2 + horizontal.x]); // (0.75 / 4), (0.25 / 4) horizontal
      pBuffer[idx] += T((threeQuarter + oneQuarter) * pBuffer[filledOffset + (vertical.y * filledCount / 2 + vertical.x]); // (0.75 / 4), (0.25 / 4)

      const vec2i diagonal = parent + vec2i(xDir, yDir);
      pBuffer[idx] += T(2 * oneQuarter * pBuffer[filledOffset + (diagonal.y * filledCount / 2 + diagonal.x]); // 2 * (0.25 / 4)
    }
  }

  if (count >= maxCount) // TODO see below!
    return;

  else
    generate_noise_revursive(pBuffer, count * count, maxCount); // TODO: handle that the count could be bigger than max count but not all values are filled in yet

}

void terrain_destroy(terrain *pTerrain)
{
  if (pTerrain == nullptr)
    return;

  lsFreePtr(&pTerrain->pTiles);
}

// TODO read/write data from/to file
