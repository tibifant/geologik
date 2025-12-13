#include "terrain.h"

lsResult terrain_init(_Out_ terrain *pTerrain, const uint16_t width, const uint16_t height)
{
  lsResult result = lsR_Success;

  pTerrain->width = width;
  pTerrain->height = height;

  LS_ERROR_CHECK(lsAlloc(&(pTerrain->pTiles), width * height));

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
      const uint16_t height = (uint16_t)(lsSin(i) * 255 + 255);

      for (size_t tt = 0; tt < tt_bedrock; tt++)
        pTerrain->pTiles[i].layerHeights[tt] = height;

      pTerrain->pTiles[i].layerHeights[tt_bedrock] = 8;
    }
  }
}

void terrain_destroy(terrain *pTerrain)
{
  if (pTerrain == nullptr)
    return;

  lsFreePtr(&pTerrain);
}

// TODO read/write data from/to file
