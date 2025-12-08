#version 150 core

attribute vec3 position;
attribute float texCoord;

uniform mat4 matrix;
uniform mat4 lastMatrix;

out float _texCoord;

void main()
{
    bool even = (gl_VertexID & 1) == 0;

    if (even)
    {
        _texCoord = 0.f;
        gl_Position = matrix * vec4(position, 1.0);
    }
    else
    {
        _texCoord = 1.f;

        vec4 current = matrix * vec4(position, 1.0);
        vec4 last = lastMatrix * vec4(position, 1.0);

        vec2 dist = current.xy - last.xy;
        
        if (dist.x == 0 && dist.y == 0)
            dist.xy = vec2(0.001);
        
        float len = length(dist);

        vec2 d = normalize(dist) * max(1, len * 5);

        gl_Position = vec4(current.xy + d, current.zw);
    }
}
