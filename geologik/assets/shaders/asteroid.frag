#version 150 core

out vec4 color;

uniform vec3 lightDir;
uniform vec3 ambient;
uniform vec3 baseColor;
uniform sampler2D normalTexture;

in vec3 _normal;
in vec2 _texCoord;

#define rX rotation.x
#define rY rotation.y
#define rZ rotation.z

void main()
{
    vec2 rotation = (texture2D(normalTexture, _texCoord).xy - vec2(0.5)) * 3.141593 * 0.5;

    mat4 rotMatX = mat4(
        1, 0, 0, 0,
        0, -cos(rX), sin(rX), 0,
        0, -sin(rX), -cos(rX), 0,
        0, 0, 0, 1
    );

    mat4 rotMatY = mat4(
        -cos(rY), 0, -sin(rY), 0,
        0, 1, 0, 0,
        sin(rY), 0, -cos(rY), 0,
        0, 0, 0, 1
    );

    mat4 rotMat = rotMatX * rotMatY;

    float light = dot(normalize((rotMat * vec4(_normal, 1)).xyz), lightDir);
    color = vec4(mix(ambient, baseColor, clamp(light * 0.75 + 0.25, 0, 1)), 1.0f);
}