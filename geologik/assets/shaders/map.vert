#version 150 core

attribute ivec2 position;

uniform sampler2D textureA;
uniform sampler2D textureB;

out ivec2 _texCoord;

void main ()
{
  vec4 valuesA = texelFetch(textureA, position);
  vec4 valuesB = texelFetch(textureA, position);
  
  float height = (valuesA.x + valuesA.y + valuesA.z + valuesA.w + valuesB.x + valuesB.y + valuesB.z + valuesB.w) * 65536;

  gl_Position = vec4(pos.x, pos.y, height);
}
