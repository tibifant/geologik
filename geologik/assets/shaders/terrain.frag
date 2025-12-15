#version 430 core

out vec4 color;

in flat uvec2 _texCoord;
in float _height;

void main()
{
  //vec4 valuesA = texelFetch(textureA, _texCoord);
  //vec4 valuesB = texelFetch(textureA, _texCoord);
//
  //if (valuesA.x) // snow
  //  color = vec4(1, 1, 1, 0);
  //else if (valuesA.y) // water
  //  color = vec4(0, 0, 0.6, 0);
  //else if (valuesA.z) // grass
  //  color = vec4(0, 0.6, 0, 0);
  //else if (valuesA.w) // soil
  //  color = vec4(0.4, 0.4, 0, 0);
  //else if (valuesB.x) // sand
  //  color = vec4(0.6, 0.4, 0.1, 0);
  //else if (valuesB.y) // limestone
  //  color = vec4(0.7, 0.7, 0.7, 0);
  //else if (valuesB.z) // stone
  //  color = vec4(0.4, 0.4, 0.4, 0);
  //else if (valuesB.w) // bedrock
  //  color = vec4(0.1, 0.1, 0.1, 0);

  color = vec4(vec3(_height * 0.001), 0); // vec2(_texCoord) * 0.01
}
