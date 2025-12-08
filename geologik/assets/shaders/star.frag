#version 150 core

out vec4 color;

uniform sampler2D texture;

in float _texCoord;

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

void main()
{
    float depth = 1.f - linearize_depth(gl_FragCoord.z, 10, 1000) / 990;
    depth *= depth;
    depth *= depth;

    color = texture2D(texture, vec2(_texCoord, 1.f - depth));

    color.rgb *= depth;
}