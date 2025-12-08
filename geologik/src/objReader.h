#pragma once

#include "core.h"
#include "queue.h"

template <typename T>
struct triangle
{
  T v[3];
};

struct objInfo_vertex
{
  vec3f position;
};

struct objInfo_triangleVertexInfo : objInfo_vertex
{
  vec3f normal;
  vec2f texCoord;
  uint8_t hasTexCoord : 1;
  uint8_t hasNormal : 1;
};

struct objInfo
{
  queue<triangle<objInfo_triangleVertexInfo>> triangles;
  queue<objInfo_vertex> vertices;
};

lsResult objReader_load(_Out_ objInfo *pInfo, const char *contents);
lsResult objReader_loadFromFile(_Out_ objInfo *pInfo, const char *filename);
