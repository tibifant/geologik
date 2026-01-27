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

void main()
{
  uint topIdx = _index - width;
  uint bottomIdx = _index + width;
  uint leftIdx = _index - 1;
  uint rightIdx = _index + 1;

  float yDiff = (float(height_from_idx(topIdx)) - float(height_from_idx(bottomIdx))) * 0.5;
  float xDiff = (float(height_from_idx(rightIdx)) - float(height_from_idx(leftIdx))) * 0.5;
  vec3 normal = normalize(vec3(xDiff, yDiff, 100 /* factor from vertex shader */));
  
  vec3 colors[] = { vec3(1, 1, 1), vec3(0, 0, 0.6), vec3(0, 0.6, 0), vec3(0.4, 0.4, 0), vec3(0.6, 0.4, 0.1), vec3(0.7, 0.7, 0.7), vec3(0.4, 0.4, 0.4), vec3(0.1, 0.1, 0.1) };
  uint mask = 0xFFFF;

  vec3 c = vec3(1, 0, 0);

  for (uint i = 0; i < 4; i++)
  {
    if ((tiles[_index].heights[i] & mask) > 0)
    {
      c = colors[i * 2];
      break;
    }
    else if ((tiles[_index].heights[i] >> 16) > 0)
    {
      c = colors[i * 2 + 1];
      break;
    }
  }

  color = vec4(c * vec3(dot(normalize(vec3(0.2, 0.6, 0.3)), normal) * 0.5 + 0.5), 1);
}
