#version 330 core
in vec2 vUV;

uniform sampler2D u_tex;
uniform bool u_hasTexture;

out vec4 FragColor;

void main()
{
    if (u_hasTexture)
    FragColor = texture(u_tex, vUV);
else
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);  // default white

}
