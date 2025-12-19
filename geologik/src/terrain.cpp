#include "terrain.h"

lsResult terrain_init(_Out_ terrain *pTerrain, const uint16_t width)
{
  lsResult result = lsR_Success;

  pTerrain->width = width;

  LS_ERROR_CHECK(lsAllocZero(&(pTerrain->pTiles), width * width));

epilogue:
  return result;
}

void generate_sin_cos(terrain *pTerrain)
{
  lsAssert(pTerrain != nullptr);

  size_t i = 0;

  for (size_t y = 0; y < pTerrain->width; y++)
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

void terrain_generate(terrain *pTerrain)
{
  lsAssert(pTerrain != nullptr);

  //generate_sin_cos(pTerrain);

  uint16_t *pNoise = nullptr;
  lsAlloc(&pNoise, pTerrain->width * pTerrain->width); // TODO errror check
  
  for (size_t tt = 6; tt < tt_bedrock; tt++)
  {
    generate_noise(pNoise, pTerrain->width);
  
    for (size_t i = 0; i < pTerrain->width * pTerrain->width; i++)
      pTerrain->pTiles[i].layerHeights[tt] = pNoise[i];
  }
  
  lsFreePtr(&pNoise);

  // TODO: set bedrock manually
  //pTerrain->pTiles[i].layerHeights[tt_bedrock] = 8;
}

void terrain_destroy(terrain *pTerrain)
{
  if (pTerrain == nullptr)
    return;

  lsFreePtr(&pTerrain->pTiles);
}

// TODO read/write data from/to file
