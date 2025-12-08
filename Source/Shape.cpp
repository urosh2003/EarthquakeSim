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

bool pointInShape(const Vec2& p, const std::vector<Vec2>& verts)
{
    bool inside = false;
    int n = verts.size();

    for (int i = 0; i < n; i++)
    {
        int j = (i - 1 + n) % n;

        float xi = verts[i].x, yi = verts[i].y;
        float xj = verts[j].x, yj = verts[j].y;

        bool intersect = ((yi > p.y) != (yj > p.y)) &&
            (p.x < (xj - xi) * (p.y - yi) / (yj - yi + 0.000001f) + xi);

        if (intersect)
            inside = !inside;
    }

    return inside;
}

