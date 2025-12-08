#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform vec2 screenSize;

void main() {
    gl_Position = vec4(
        vertex.x * 2.0 / screenSize.x - 1.0,
        1.0 - vertex.y * 2.0 / screenSize.y,
        0.0, 1.0
    );
    TexCoords = vertex.zw;
}