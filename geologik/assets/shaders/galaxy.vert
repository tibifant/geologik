#version 150 core

attribute vec3 position;
attribute vec2 texCoord;

uniform mat4 matrix;

out vec2 _texCoord;

void main()
{
    _texCoord = texCoord;

    gl_Position = matrix * vec4(position, 1.0);
}