#version 150 core

attribute vec2 position;

uniform vec2 scale;
uniform vec2 offset;

out vec2 _texCoord;

void main()
{
    _texCoord = position;

    vec2 pos = position;

    pos *= scale;
    pos += offset;

    pos.y = 1 - pos.y;

    gl_Position = vec4(pos * 2 - 1, 1.0, 1.0);
}