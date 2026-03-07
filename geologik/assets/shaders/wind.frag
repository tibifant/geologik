#version 430 core

in vec4 windData;
in uvec2 scaledPos;

uniform uint terrainWidth;
uniform uint noiseZ;
uniform sampler3D noise;

out vec4 color;

void main()
{
  //float windStrength = clamp(length(windData.xy) * 12, 0, 1);
  //float alpha = 1 - windStrength;
  //alpha = alpha * alpha;
  //alpha = 1 - alpha;

  //color = vec4(mix(vec3(0.2, 0.75, 0.9), vec3(1), windStrength), max(alpha - 0.2, 0) * 0.7);
  //color = vec4(windData.xy * 5 + 0.5, 0.5, 0.7);
  
  ivec3 noisePos = ivec3(scaledPos, noiseZ);
  bool foundDot = false;

  for (uint i = 0; i < 5; i++)
  {
    if (noise[noisePos.y * terrainWidth + noisePos.x] > 10000) // what is the max val of the noise?
    {
      foundDot = true;
      break; 
    }

    noisePos.xy += ivec2(wind.xy);

    if (noisePos.x < 0 || noisePos.y < 0 || noisePos.x > terrainWidth - 1 || noisePos.y > terrainWidth - 1)
      break;
  }

  color = vec4(vec3(0), float(foundDot));
}
