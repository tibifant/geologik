#version 450

vec2 positions[6] = vec2[](
  vec2(0, 0),
  vec2(1, 0),
  vec2(0, 1),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1)
);

uniform mat4 inverseMatrix;

layout(location = 0) out vec3 fDirection;

void main()
{
  vec2 uv = positions[gl_VertexID];
  vec2 pos = uv * 2 - vec2(1);

  fDirection = (inverseMatrix * vec4(pos, 1, 1)).xyz;

  gl_Position = vec4(pos, 1, 1);
}
