#version 150 core

out vec4 color;

uniform sampler2D texture;
uniform float index;

in vec2 _texCoord;

void main()
{
    vec2 coord = _texCoord;
    coord.x *= 0.1;
    coord.x += 0.1 * index;

    color = texture2D(texture, coord);
}