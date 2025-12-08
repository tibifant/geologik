#version 150 core

attribute vec2 position;
attribute vec2 offset;
attribute float texCoord;

uniform mat4 matrix;
uniform vec2 scale;
uniform float scaleAlphaFactor;

out float _alpha;
out vec2 _texCoord;

void main()
{
    _alpha = texCoord;
    _texCoord = position;

    float angle = float(gl_InstanceID) * 0.025;

    vec2 p = position - vec2(0.5);
    p = vec2(cos(angle) * p.x + sin(angle) * p.y, -sin(angle) * p.x + cos(angle) * p.y);
    
    vec2 pos = offset + p * mix(scale, scale * scaleAlphaFactor, 1 - texCoord);

    gl_Position = matrix * vec4(pos, 1.0, 1.0);
}