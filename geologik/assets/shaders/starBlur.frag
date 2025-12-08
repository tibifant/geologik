#version 150 core

out vec4 color;

uniform sampler2D texture;
uniform sampler2D weight;
uniform vec2 invSize;
uniform vec2 weightSize;

in vec2 _texCoord;

void main()
{
    vec3 blur = vec3(0);
    vec3 wsum = vec3(0);
    vec2 halfWS = weightSize * 0.5;
    vec2 iWS = vec2(1) / weightSize;

    for (float y = 0; y <= weightSize.y; y += 2)
    {
        for (float x = 0; x <= weightSize.x; x += 2)
        {
            vec2 xy = vec2(x, y);
            vec3 w = texture2D(weight, xy * iWS).rgb;
            vec3 c = texture2D(texture, _texCoord + (xy - halfWS) * invSize).rgb;
            blur += c * w;
            wsum += w;
        }
    }

    blur /= wsum;
    blur = clamp(blur * 8.f, 0, 1);
    blur = vec3(1) - blur;
    blur *= blur;
    blur = vec3(1) - blur;
    
    color = vec4(blur * 0.5f, 1);
}