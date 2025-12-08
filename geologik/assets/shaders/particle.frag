#version 150 core

out vec4 color;

uniform sampler2D texture;
uniform sampler2D alphaTexture;
uniform float maxAlpha;
uniform float minColor;

in float _alpha;
in vec2 _texCoord;

void main()
{
    float f = texture2D(texture, _texCoord).x;

    color = vec4(vec3(f * (1 - minColor) + minColor), f * maxAlpha);
    color *= texture2D(alphaTexture, vec2(_alpha, 0.5));
}