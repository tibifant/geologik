#version 150 core

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;

attribute mat4 matrix; // per instance.
attribute vec3 rotation; // per instance.

uniform mat4 vp;

out vec3 _normal;
out vec2 _texCoord;

#define rX rotation.x
#define rY rotation.y
#define rZ rotation.z

void main()
{
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

    mat4 rotMatZ = mat4(
        -cos(rZ), sin(rZ), 0, 0,
        -sin(rZ), -cos(rZ), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    mat4 rotMat = rotMatX * rotMatY * rotMatZ;

    _normal = (rotMat * vec4(normal, 1.0)).xyz;
    _texCoord = texCoord;
    _texCoord.y = 1 - _texCoord.y;

    gl_Position = vp * matrix * rotMat * vec4(position, 1.0);
}
