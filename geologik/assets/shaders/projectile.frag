#version 150 core

out vec4 color;

uniform sampler2D texture;
uniform sampler2D spaceTex;
uniform vec3 lightDir;
uniform vec3 ambient;

in vec3 _normal;
in vec2 _texCoord;
in vec2 _pos;

void main()
{
    vec4 tex = texture2D(texture, _texCoord);
    float light = dot(normalize(_normal), lightDir);
    tex.a = clamp(tex.a, 0.5, 0.9);
    color.a = 1;
    color.rgb = mix(ambient, tex.rgb, clamp(light * 3 - 0.75, 0, 1));
    color.rgb = mix(texture2D(spaceTex, mod(_pos + _normal.rb * 0.5 + 0.5, 1)).rgb + tex.rgb * 0.5, color.rgb, tex.a);
}