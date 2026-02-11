#version 430 core

struct tile
{
  uint heights[4];
};

layout(binding = 0, std430) buffer data 
{
  tile tiles[];
};

uniform image2D texture;

uniform vec2 texCoord;

// in comp: read and interpolate?
// here: set

void main()
{
  // sample height & normals for average
  vec2 samplePos = texCoord * vec2(8, 8); // 2,2 6,2 2,6 6,6
  vec2 samplePosA = samplePos + vec2(2, 2);
  vec2 samplePosB = samplePos + vec2(6, 2);
  vec2 samplePosC = samplePos + vec2(2, 6);
  vec2 samplePosD = samplePos + vec2(6, 6);

  // calc temp from that
  // mix with last temp
  // set temp
}

// for a specific tile: interpolate
