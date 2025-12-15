#version 430 core

layout(location = 0) in uvec2 position;

uniform uvec2 offset;
uniform uint width;
uniform mat4x4 vp;

out flat uvec2 _texCoord;
out float _height;

layout(binding = 0, std430) buffer data 
{
  uint vals[];
};

void main ()
{
  uvec2 pos = position + offset;
  uint idx = (pos.y * width + pos.x) * 4;
  
  uint mask = 0xFFFF;
  uint height = 0; 

  for (uint i = 0; i < 4; i++)
  {
    uint j = idx + i;
    height += vals[j] & mask;
    height += vals[j] >> 16;
  }

  _height = float(height);
  _texCoord = pos;
  vec4 pos4 = vec4(vec2(pos), float(height) * 0.01, 1.0);

  gl_Position = pos4 * vp;
}
