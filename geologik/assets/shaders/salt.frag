#version 150 core

out vec4 color;

uniform vec3 lightDir;
uniform vec3 ambient;
uniform sampler2D texture;
uniform sampler2D reflectionTexture;
uniform float reflectivity;

in vec3 _normal;
in vec2 _texCoord;

const float PI = 3.141592653589793f;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}

void main()
{
    float light = dot(normalize(_normal), lightDir);

    vec2 reflectionDirection = vec2(atan2(_normal.y, _normal.x), atan2(length(_normal.xy), _normal.z)) / vec2(PI * 2, PI) + 0.5;
    vec3 reflected = texture2D(reflectionTexture, reflectionDirection).rgb;
    vec3 tex = texture2D(texture, _texCoord).rgb;

    color = vec4(mix(ambient, tex, clamp(light * 1.25 + 0.25, 0, 1)) + reflected * reflectivity, 1.0f);
}