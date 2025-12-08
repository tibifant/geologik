#version 150 core

out vec4 color;

uniform sampler2DMS texture;
uniform sampler2DMS depth;
uniform int sampleCount;
uniform sampler2D tiny;
uniform vec2 invTinySize;

in vec2 _texCoord;

vec4 sampledSum(sampler2DMS s, int count, vec2 pos)
{
    ivec2 position = ivec2(pos * textureSize(s));
    vec4 c = vec4(0);

    for (int i = 0; i < count; i++)
        c += texelFetch(s, position, i);

    return c;
}

float sampledSumX(sampler2DMS s, int count, vec2 pos)
{
    ivec2 position = ivec2(pos * textureSize(s));
    float c = 0;

    for (int i = 0; i < count; i++)
        c += texelFetch(s, position, i).x;

    return c;
}

float sampledSumXwoffs(sampler2DMS s, int count, vec2 pos, ivec2 offset)
{
    ivec2 position = ivec2(pos * textureSize(s)) + offset;
    float c = 0;

    for (int i = 0; i < count; i++)
        c += texelFetch(s, position, i).x;

    return c;
}

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float depth_at(vec2 pos)
{
    float d = sampledSumX(depth, sampleCount, pos) / float(sampleCount);
    float ld = linearize_depth(d, 1, 50) / 49.0;

    return d;
}

float depthwoffs_at(vec2 pos, ivec2 offset)
{
    float d = sampledSumXwoffs(depth, sampleCount, pos, offset) / float(sampleCount);
    float ld = linearize_depth(d, 1, 50) / 49.0;

    return d;
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

void main()
{
    float ld = depth_at(_texCoord);
    float diff = 0;
    float wsum = 0;
    
    for (int y = -2; y <= 2; y++)
    {
        for (int x = -2; x <= 2; x++)
        {
            if (x == 0 && y == 0)
                continue;

            float w = 1.0 / length(vec2(x, y));
            diff += min(abs(ld - depthwoffs_at(_texCoord, ivec2(x, y))), 0.001) * w;
            wsum += w;
        }
    }

    diff *= 1000;
    diff /= wsum;
    diff *= diff;
    diff *= ld;
    diff = clamp(diff * 1.1 - 0.1, 0, 1);

    vec3 col = sampledSum(texture, sampleCount, _texCoord).rgb / float(sampleCount);
    vec3 hsv = rgb2hsv(col);
    hsv.z = clamp(hsv.z * (1 + diff * 6) + diff * 0.2, 0, 1);
    hsv.y = clamp(mix(hsv.y * (1 + diff * 3), hsv.y * (1 - diff), hsv.z), 0, 1);

    color = vec4(hsv2rgb(hsv), 1);

    vec3 blur = vec3(0);
    wsum = 0;
    
    for (int y = -6; y <= 6; y += 3)
    {
        for (int x = -6; x <= 6; x += 3)
        {
            vec2 xy = vec2(float(x), float(y));
            float w = 2.0 / max(length(xy), 2);
            blur += texture2D(tiny, _texCoord + xy * invTinySize).rgb * w;
            wsum += w;
        }
    }
    
    blur /= wsum;
    blur *= blur;
    blur = vec3(1) - blur;
    blur *= blur;
    blur = vec3(1) - blur;
    blur -= 0.0025;
    blur *= 0.4;

    color.rgb += blur;
}