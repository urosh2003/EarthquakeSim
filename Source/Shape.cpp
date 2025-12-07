#include "../Header/Shape.h";


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>


std::vector<Vec2> makeCircle(int segments)
{
    std::vector<Vec2> v;
    for (int i = 0; i < segments; i++)
    {
        float a = (float)i / segments * 6.283185f;
        v.push_back({ cosf(a), sinf(a) });
    }
    return v;
}

std::vector<Vec2> makeSquare()
{
    return { {-1,-1}, {1,-1}, {1,1}, {-1,1} };
}

std::vector<Vec2> makeTriangle()
{
    return {
        {0,1},
        {-0.866f,-0.5f},
        {0.866f,-0.5f}
    };
}


std::vector<Vec2> makeRectangle()
{
    return { {-2,-1}, {2,-1}, {2,1}, {-2,1} };
}


std::vector<Vec2> makeEllipse(int segments)
{
    std::vector<Vec2> v;
    for (int i = 0; i < segments; i++)
    {
        float a = (float)i / segments * 6.283185f;
        v.push_back({ 2 * cosf(a), 1 * sinf(a) });
    }
    return v;
}

std::vector<Vec2> makeHexagon()
{
    std::vector<Vec2> v;
    for (int i = 0; i < 6; i++)
    {
        float a = i / 6.0f * 6.283185f;
        v.push_back({ cosf(a), sinf(a) });
    }
    return v;
}
