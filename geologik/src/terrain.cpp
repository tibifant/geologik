#include "terrain.h"

lsResult map_init(_Out_ terrain *pTerrain, const uint16_t width, const uint16_t height)
{
  lsResult result = lsR_Success;

  pTerrain->width = width;
  pTerrain->height = height;

  LS_ERROR_CHECK(lsAlloc(&(pTerrain->pTiles), width * height));

epilogue:
  return result;
}

void map_generate_terrain(terrain *pTerrain)
{
  lsAssert(pTerrain != nullptr);

  for (size_t y = 0; y < pTerrain->height; y++)
  {
    for (size_t x = 0; x < pTerrain->width; x++)
    {
      const size_t i = y * pTerrain->width + pTerrain->width;
      const uint16_t height = lsSin(i) * lsMaxValue<uint16_t>();

      for (size_t tt = 0; tt < tt_count; tt++)
        pTerrain->pTiles[i].layerHeights[tt] = height;
    }
  }
}

// TODO read/write data from/to file
