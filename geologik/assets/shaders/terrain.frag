#version 430 core

struct tile
{
  uint heights[4];
};

layout(binding = 0, std430) buffer data 
{
  tile tiles[];
};

uniform uint width;
uniform vec3 sunDir;

in flat uint _index;

out vec4 color;

uint height_from_idx(uint idx)
{
  uint mask = 0xFFFF;
  uint height = 0;

  for (uint i = 0; i < 4; i++)
  {
    height += tiles[idx].heights[i] & mask;
    height += tiles[idx].heights[i] >> 16;
  }

  return height;
}

vec3 color_from_height(uint terrainType, uint height)
{
  vec3 colorsThick[] = { vec3(1, 1, 1),       vec3(0.1, 0.3, 0.6),    vec3(0.5, 0.75, 0.3), vec3(0.32, 0.24, 0.2), vec3(0.65, 0.56, 0.33), vec3(0.65, 0.62, 0.52), vec3(0.4, 0.4, 0.4), vec3(0.1, 0.1, 0.1) };
  vec3 colorsThin[] = { vec3(0.8, 0.9, 0.95), vec3(0.2, 0.4, 0.65), vec3(0.6, 0.85, 0.4), vec3(0.47, 0.39, 0.35), vec3(0.85, 0.76, 0.53), vec3(0.85, 0.82, 0.62), vec3(0.65, 0.65, 0.65), vec3(0.1, 0.1, 0.1) };

  return mix(colorsThin[terrainType], colorsThick[terrainType], clamp(float(height) * 0.05, 0, 1));
}

void main()
{
  uint topIdx = _index - width;
  uint bottomIdx = _index + width;
  uint leftIdx = _index - 1;
  uint rightIdx = _index + 1;

  float yDiff = (float(height_from_idx(topIdx)) - float(height_from_idx(bottomIdx))) * 0.5;
  float xDiff = (float(height_from_idx(rightIdx)) - float(height_from_idx(leftIdx))) * 0.5;
  vec3 normal = normalize(vec3(xDiff, yDiff, 100 /* factor from vertex shader */));
 
  uint mask = 0xFFFF;

  vec3 c = vec3(1, 0, 0);

  for (uint i = 0; i < 4; i++)
  {
    if ((tiles[_index].heights[i] & mask) > 0)
    {
      c = color_from_height(i * 2, (tiles[_index].heights[i] & mask));
      break;
    }
    else if ((tiles[_index].heights[i] >> 16) > 0)
    {
      c = color_from_height(i * 2 + 1, ((tiles[_index].heights[i] >> 16)));
      break;
    }
  }

  color = vec4(c * vec3(dot(sunDir, normal) * 0.5 + 0.5), 1);
}
