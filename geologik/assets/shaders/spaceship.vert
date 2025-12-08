#version 150 core

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;

uniform mat4 matrix;
uniform mat4 rotMat;

out vec3 _normal;
out vec2 _texCoord;
out vec2 _pos;

void main()
{
    _normal = (rotMat * vec4(normal, 1.0)).xyz;
    _texCoord = texCoord;
    _texCoord.y = 1 - _texCoord.y;

    gl_Position = matrix * vec4(position, 1.0);
    _pos = gl_Position.xy;
}
