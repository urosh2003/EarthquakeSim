#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Vec2 {
    float x, y;
};

struct Shape {
    Vec2 position;
    GLuint vao;
    GLuint vbo;
    GLsizei vertexCount;
    float rotation;
    float scale;
};

std::vector<Vec2> makeCircle(int segments = 60);

std::vector<Vec2> makeSquare();

std::vector<Vec2> makeTriangle();

std::vector<Vec2> makeRectangle();

std::vector<Vec2> makeEllipse(int segments = 60);

std::vector<Vec2> makeHexagon();