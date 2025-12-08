#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Vec2 {
    float x, y;
    Vec2 operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
    Vec2 operator*(float f) const { return { x * f, y * f }; }
};

struct Shape {
    Vec2 position;
    GLuint vao;
    GLuint vbo;
    GLsizei vertexCount;
    float rotation;
    float scale;
    unsigned int texture = 0;
    bool hasTexture = false;
    std::vector<Vec2> baseVerts;
};

std::vector<Vec2> makeCircle(int segments = 60);

std::vector<Vec2> makeSquare();

std::vector<Vec2> makeTriangle();

std::vector<Vec2> makeRectangle();

std::vector<Vec2> makeEllipse(int segments = 60);

std::vector<Vec2> makeHexagon();
bool pointInShape(const Vec2& p, const std::vector<Vec2>& verts);