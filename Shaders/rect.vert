#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
out vec2 chTex;

uniform float uX;
uniform float uY;
uniform float uS; // skaliranje geometrijskog tela množenjem
uniform int goingRight;

void main()
{
    gl_Position = vec4(inPos.x + uX, inPos.y * uS + uY, 0.0, 1.0);
    if(goingRight == 0)
    {
        chTex = inTex;
    }
    else
    {
        chTex[0] = 1 - inTex[0];
        chTex[1] = inTex[1];
    }
}