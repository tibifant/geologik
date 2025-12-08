#version 150 core

attribute vec2 position;

out vec2 _texCoord;

void main()
{
    _texCoord = position;

    gl_Position = vec4(position * 2 - 1, 1.0, 1.0);
}