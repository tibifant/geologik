#version 430 core

in vec4 windData;

out vec4 color;

void main()
{
  float windStrength = clamp(length(windData.xy) * 12, 0, 1);
  float alpha = 1 - windStrength;
  alpha = alpha * alpha;
  alpha = 1 - alpha;

  color = vec4(mix(vec3(0.2, 0.75, 0.9), vec3(1), windStrength), max(alpha - 0.2, 0) * 0.7);
  //color = vec4(windData.xy * 5 + 0.5, 0.5, 0.7);
}
