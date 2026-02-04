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

lsResult terrain_generate(terrain *pTerrain)
{
  lsResult result = lsR_Success;

  lsAssert(pTerrain != nullptr);

  constexpr int32_t MinHeight[tt_bedrock] = {
    -50, // tt_snow 
    -100, // tt_water 
    0, // tt_grass
    -100, // tt_soil 
    -100, // tt_sand 
    -16000, // tt_limestone 
    100 // tt_stone 
  };
  
  constexpr int32_t MaxHeight[tt_bedrock] = {
    100, // tt_snow
    50, // tt_water
    10, // tt_grass
    100, // tt_soil
    100, // tt_sand
    16000, // tt_limestone
    32000 // tt_stone
  };
  
  const size_t terrainSize = pTerrain->width * pTerrain->width;

  uint16_t *pNoise = nullptr;
  uint16_t *pTotalHeight = nullptr;

  LS_ERROR_CHECK(lsAllocZero(&pNoise, terrainSize));
  LS_ERROR_CHECK(lsAllocZero(&pTotalHeight, terrainSize));
  
  for (int8_t tt = tt_stone; tt >= 0; tt--)
  {
    //if (tt == tt_water)
      //continue;

    generate_noise(pNoise, pTerrain->width);

    FILE *pFile = fopen(sformat("C:\\data\\noise", tt, ".raw"), "wb");
    fwrite(pNoise, sizeof(uint16_t), terrainSize, pFile);
    fflush(pFile);
    fclose(pFile);

    for (size_t i = 0; i < terrainSize; i++)
    {
      if (tt == tt_sand && pTerrain->pTiles[i].layerHeights[tt_limestone] == 0)
      {
        pTerrain->pTiles[i].layerHeights[tt] = 0;
        continue;
      }
      if (tt == tt_soil && pTerrain->pTiles[i].layerHeights[tt_sand] > 0)
      {
        pTerrain->pTiles[i].layerHeights[tt] = 0;
        continue;
      }
      else if (tt == tt_grass && (pTerrain->pTiles[i].layerHeights[tt_soil] < 5 || pTotalHeight[i] > 20000))
      {
        pTerrain->pTiles[i].layerHeights[tt] = 0;
        continue;
      }
      else if (tt == tt_snow && pTotalHeight[i] < 18000)
      {
        pTerrain->pTiles[i].layerHeights[tt] = 0;
        continue;
      }

      const int32_t mappedVal = (int32_t)((float_t)(pNoise[i]) / lsMaxValue<uint16_t>() * (MaxHeight[tt] + lsAbs(MinHeight[tt])) - lsAbs(MinHeight[tt]));
      const uint16_t val = (uint16_t)(lsClamp(mappedVal, int32_t(0), (int32_t)(lsMaxValue<uint16_t>())));

      pTerrain->pTiles[i].layerHeights[tt] = val;

      pTotalHeight[i] += val;
    }
  }

  lsFreePtr(&pNoise);

  for (size_t i = 0; i < pTerrain->width * pTerrain->width; i++)
    pTerrain->pTiles[i].layerHeights[tt_bedrock] = 8;

epilogue:
  return result;
}

void terrain_destroy(terrain *pTerrain)
{
  if (pTerrain == nullptr)
    return;

  lsFreePtr(&pTerrain->pTiles);
}

// TODO read/write data from/to file
