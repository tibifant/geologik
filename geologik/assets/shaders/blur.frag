#version 150 core

out vec4 color;

uniform sampler2DMS texture;

in vec2 _texCoord;

vec4 sampledSumWOffs(sampler2DMS s, vec2 pos, ivec2 offset)
{
    ivec2 position = ivec2(pos * textureSize(s) + offset) * 16;
    vec4 c = vec4(0);
    
    for (int y = 0; y < 16; y += 4)
        for (int x = 0; x < 16; x += 4)
            c += texelFetch(s, position + ivec2(x, y), 0);
    
    return c;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 clamp_by_brightness(vec3 c)
{
    return c * clamp((max(c.r - 0.5, 0) + max(c.g - 0.5, 0) + max(c.b - 0.5, 0)) * 10, 0, 1);
}

void main()
{
    vec3 c = vec3(0);
    float wsum = 0;
    
    for (int y = -4; y <= 4; y++)
    {
        for (int x = -4; x <= 4; x++)
        {
            float w = 1.0 / max(abs(x) + abs(y), 1);
            c += sampledSumWOffs(texture, _texCoord, ivec2(x, y)).rgb * w;
            wsum += w;
        }
    }

    c /= (wsum * 4.f * 4.f);

    c = rgb2hsv(c);
    c.y = mix(c.y, c.y * 0.25, max(c.z * 2 - 1, 0));
    c = hsv2rgb(c);

    c *= 3;

    color = vec4(c, 1);
}