#version 430 core

struct tile
{
  uint heights[4];
};

layout(binding = 0, std430) buffer data 
{
  tile tiles[];
};

in flat uint _index;
in float _height;

out vec4 color;

void main()
{
  vec4 colors[] = { vec4(1, 1, 1, 0), vec4(0, 0, 0.6, 0), vec4(0, 0.6, 0, 0), vec4(0.4, 0.4, 0, 0), vec4(0.6, 0.4, 0.1, 0), vec4(0.7, 0.7, 0.7, 0), vec4(0.4, 0.4, 0.4, 0), vec4(0.1, 0.1, 0.1, 0) };
  uint mask = 0xFFFF;

  vec4 c = vec4(1, 0, 0, 0);

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

  color = vec4(0, 0, 1 -_height * 0.000009, 0); //c; //* _height * 0.001;
}
