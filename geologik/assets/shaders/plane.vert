#version 150 core

attribute vec2 position;

uniform mat4 matrix;

out vec2 _texCoord;

void main()
{
    _texCoord.x = position.x;
    _texCoord.y = 1 - position.y;

    gl_Position = matrix * vec4(position, 0.0, 1.0);
}