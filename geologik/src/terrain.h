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
  uint16_t height;

  tile *pTiles;
};

lsResult terrain_init(_Out_ terrain *pTerrain, const uint16_t width, const uint16_t height);
void terrain_generate(terrain *pTerrain);
