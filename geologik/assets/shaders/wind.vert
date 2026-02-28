#version 430 core

layout(location = 0) in uvec2 position;

uniform uvec2 offset;
uniform float scale;
uniform mat4x4 vp;
uniform sampler2D texture;

out vec4 windData;

void main ()
{
  uvec2 pos = min(position + offset, uvec2(255));
  vec4 data = texelFetch(texture, ivec2(pos), 0);
  windData = data;

  vec4 pos4 = vec4(vec2(pos) * scale, float(data.z) * 0.01 + 5, 1.0);

  gl_Position = pos4 * vp;
}
