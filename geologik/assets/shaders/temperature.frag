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
uniform float lerpFactor;
uniform uint width;
uniform vec3 sunDir;

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
  // sample height & normals for average
  vec2 samplePos = texCoord * vec2(8, 8);
  vec2 samplePosTop = samplePos + vec2(4, 2);
  vec2 samplePosRight = samplePos + vec2(6, 4);
  vec2 samplePosBottom = samplePos + vec2(4, 6);
  vec2 samplePosLeft = samplePos + vec2(2, 4);

  uint heightTop = height_from_idx(samplePosTop.y * width + samplePosTop.x);
  uint heightRight = height_from_idx(samplePosRight.y * width + samplePosRight.x);
  uint heightBottom = height_from_idx(samplePosBottom.y * width + samplePosBottom.x);
  uint heightLeft = height_from_idx(samplePosLeft.y * width + samplePosLeft.x);

  uint avgHeight = (heightTop + heightRight + heightBottom + heightLeft) / 4;

  float yDiff = (float(heightTop) - float(heightBottom)) * 0.5; // is this an accaptable approximation
  float xDiff = (float(heightRight) - float(heightLeft)) * 0.5;
  vec3 avgNorm = normaliz(vec3(xDiff, yDiff, 100));

  // calc temp from that
  const float minTemp = -7;
  const float maxTemp = 22;
  const float sunMaxTemp = 8;

  const uint maxHeight = 13000;
  float height = clamp(float(avgHeight) - 10000, float(0), float(maxHeight));

  float temp = mix(minTemp, maxTemp, 1 - height * (1.0 / maxHeight));
  float sunAllignment = dot(avgNorm, sunDir);

  if (sunAllignment > 0)
    temp += sunAllignment * sunMaxTemp;

  // mix with last temp
  float lastTemp = texelFetch(texture, texCoord);
  temp = mix(temp, lastTemp, lerpFactor);

  // set new temp
  imageStore(texture, texCoord, temp);
}
