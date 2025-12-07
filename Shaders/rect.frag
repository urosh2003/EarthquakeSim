#version 330 core

in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex;
uniform sampler2D uTex1;

uniform bool hasHat;


void main()
{
    if(hasHat)
    {
        vec4 col1 = texture(uTex, chTex);
        vec4 col2 = texture(uTex1, chTex);
        if(col2.a == 1)
        {
            outCol = col2;
        }
        else
        {
            outCol = col1;
        }
    }
    else
    {
        outCol = texture(uTex, chTex);
    }

} 