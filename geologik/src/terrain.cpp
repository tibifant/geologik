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

template <typename T>
void generate_noise_recursive(T *pBuffer, const size_t filledCount, const size_t maxCount)
{
  // TODO influence factor to prefer starting values

  const size_t offset = maxCount - filledCount * filledCount;
  const size_t filledCountOffset = maxCount - filledCount;

  for (size_t y = 0; y < filledCount; y++)
  {
    for (size_t x = 0; x < filledCount; x++)
    {
      // go one tile in all directions
      static const vec2i[] dirs = { vec2i(-1, -1), vec2i(0, -1), vec2i(1, -1), vec2i(-1, 0), vec2i(0, 0), vec2i(1, 0), vec2i(1, 1), vec2i(0, 1), vec2i(1, 1) };

      for (size_t i = 0; i < LS_ARRAYSIZE(dirs); i++)
      {
        const vec2i pos = vec2i(x, y) + dirs[i];

        // if out of bounds
        if (pos.x < 0 || pos.x > filledCount || pos.y < 0 || pos.y > filledCount)
          continue;

        // which start tile are we in?
        const vec2u paretnPos = (vec2u)(pos / 2); // floored coordiantes

        // determine influence by tile
        

        // set value based on tiles 
      }
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
