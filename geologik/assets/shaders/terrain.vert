#version 430 core

layout(location = 0) in uvec2 position;

uniform uvec2 offset;
uniform uint width;

out vec2 _texCoord;

layout(binding = 0, std430) buffer data 
{
  uint vals[];
};


void main ()
{
  uvec2 pos = position + offset;
  uint idx = (pos.y * width + pos.x) * 4;
  
  uint mask = 0XFFFF;
  uint height = 0; 

  for (uint i = 0; i < 4; i++)
  {
    uint j = idx + i;
    height += vals[j] & mask;
    height += vals[j] >> 16;
  }

  gl_Position = vec4(pos, height, 1.0);
}
