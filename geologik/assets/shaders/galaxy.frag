#version 150 core

out vec4 color;

uniform sampler2D texture;

in vec2 _texCoord;

void main()
{
    color = texture2D(texture, _texCoord) * 0.2;
}