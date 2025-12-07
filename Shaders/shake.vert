#version 330 core

layout(location = 0) in vec2 aPos;

uniform vec2 u_position;
uniform float u_rotation;
uniform float u_scale;
uniform float u_aspect_ratio;
uniform vec2 u_shake_offset;

void main()
{
    float c = cos(radians(u_rotation));
    float s = sin(radians(u_rotation));

    vec2 p = aPos * u_scale;
    p = vec2(
        p.x * c - p.y * s,
        p.x * s + p.y * c
    );

    // move to world position
    p += u_position + u_shake_offset ;
    p.x /= u_aspect_ratio;

    gl_Position = vec4(p, 0.0, 1.0);
}
