#version 430 core

layout(location = 0) in uvec2 position;

uniform uvec2 offset;
uniform uint width;
uniform mat4x4 vp;

out flat uint _index;

struct tile
{
  uint heights[4];
};

layout(binding = 0, std430) buffer data 
{
  tile tiles[];
};

void main ()
{
  uvec2 pos = min(position + offset, uvec2(width));
  uint idx = pos.y * width + pos.x;
  
  uint mask = 0xFFFF;
  uint height = 0; 

  for (uint i = 0; i < 4; i++)
  {
    height += tiles[idx].heights[i] & mask;
    height += tiles[idx].heights[i] >> 16;
  }

  _index = idx;
  vec4 pos4 = vec4(vec2(pos), float(height) * 0.01, 1.0);

  gl_Position = pos4 * vp;
}
