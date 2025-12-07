#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "../Header/Util.h"
#include "../Header/Shape.h"


std::vector<Shape> shapes;

float screenWidth;
float screenHeight;
float aspectRatio = 1.0f;
bool done = false;


bool isShaking;
double currentShakeDuration;
double maxShakeDuration = 0.5;
double shakeIntensity = 0.2;
double shakeSpeed = 0.5;

double shakePeak = 0.3;
double shakeFalloff = 0.4;

Vec2 screenToWorld(double mx, double my)
{
    float x = (float)mx / screenWidth * 2.0f - 1.0f;
    float y = 1.0f - (float)my / screenHeight * 2.0f;

    x *= aspectRatio;

    return { x, y };
}


Shape createShape(std::vector<float>& verts) {
    Shape s;

    glGenVertexArrays(1, &s.vao);
    glBindVertexArray(s.vao);

    glGenBuffers(1, &s.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,          // attribute index in shader
        2,          // vec2
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    s.vertexCount = verts.size() / 2;

    glBindVertexArray(0);

    // Random scale and rotation:
    s.scale = 0.5f + (rand() % 100 / 100.0f); // 0.5 – 1.5
    s.rotation = (float)(rand() % 360);

    return s;
}

void uploadShape(Shape& s, const std::vector<Vec2>& verts)
{
    std::vector<float> flat;
    flat.reserve(verts.size() * 2);
    for (auto& v : verts)
    {
        flat.push_back(v.x);
        flat.push_back(v.y);
    }

    glGenVertexArrays(1, &s.vao);
    glBindVertexArray(s.vao);

    glGenBuffers(1, &s.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, flat.size() * sizeof(float), flat.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    s.vertexCount = (GLsizei)verts.size();
}



void mouseClick(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        Vec2 pos = screenToWorld(mx, my);

        Shape s;
        s.position = pos;
        s.rotation = rand() % 360;
        s.scale = 0.05f + (rand() % 100) / 1000.0f;

        std::vector<Vec2> verts;

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            verts = makeCircle();
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            verts = makeSquare();
        else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            verts = makeTriangle();
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            verts = makeRectangle();
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            verts = makeEllipse();
        else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            verts = makeHexagon();
        else
            return;

        uploadShape(s, verts);
        shapes.push_back(s);
    }
}

int seed1;
int seed2;
int seed3;
int seed4;
int seed5;
int seed6;

void keyboardPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        done = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !isShaking) {
        isShaking = true;
        currentShakeDuration = 0;
        seed1 = rand() % 10000;
        seed2 = rand() % 10000;
        seed3 = rand() % 10000;

        seed4 = rand() % 10000;
        seed5 = rand() % 10000;
        seed6 = rand() % 10000;
        std::cout << seed1 << std::endl;
        std::cout << seed2 << std::endl;
        std::cout << seed3 << std::endl;
        std::cout << seed4 << std::endl;
        std::cout << seed5 << std::endl;
        std::cout << seed6 << std::endl;

    }
}


Vec2 handleShake(double deltaTime)
{
    if (!isShaking)
        return { 0, 0 };

    currentShakeDuration += deltaTime;

    if (currentShakeDuration > maxShakeDuration)
    {
        isShaking = false;
    }

    float bigShakeX = (float)cos((currentShakeDuration + seed1)* shakeSpeed * 100) * shakeIntensity;
    float mediumShakeX = (float)cos((currentShakeDuration + seed2) * shakeSpeed * 1000) * shakeIntensity/10;
    float smallShakeX = (float)cos((currentShakeDuration + seed3) * shakeSpeed * 10000) * shakeIntensity/100;

    float totalShakeX = bigShakeX + mediumShakeX + smallShakeX; 

    float bigShakeY = (float)cos((currentShakeDuration + seed4) * shakeSpeed * 100) * shakeIntensity;
    float mediumShakeY = (float)cos((currentShakeDuration + seed5) * shakeSpeed * 1000) * shakeIntensity/10;
    float smallShakeY = (float)cos((currentShakeDuration + seed6) * shakeSpeed * 10000) * shakeIntensity/100;

    float totalShakeY = bigShakeY + mediumShakeY + smallShakeY; 


    float shakePercent = currentShakeDuration / maxShakeDuration; 

    float currentShakeIntensity = 1;

    if (shakePercent < shakePeak)
    {
        currentShakeIntensity = pow(2, shakePercent/shakePeak) - 1;
    }
    else if(shakePercent > shakeFalloff)
    {
        currentShakeIntensity = pow(1 - shakePercent, 2);
    }

    return { totalShakeX * currentShakeIntensity, totalShakeY * currentShakeIntensity };
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    screenWidth = mode->width;
    screenHeight = mode->height;
    aspectRatio = screenWidth / screenHeight;
    // Fullscreen window
    GLFWwindow* window = glfwCreateWindow(
        screenWidth,
        screenHeight,
        "My Fullscreen Window",
        monitor,     // monitor → fullscreen
        nullptr      // no shared window
    );

    glfwMakeContextCurrent(window);
    GLFWcursor* myCursor = loadImageToCursor("Resources/cursor.png");
    if (myCursor) {
        glfwSetCursor(window, myCursor);
    }

    glfwSetMouseButtonCallback(window, mouseClick);
    glfwSetKeyCallback(window, keyboardPress);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    unsigned int shader = createShader("Shaders/shake.vert", "Shaders/shake.frag");
    glUseProgram(shader);

    double targetFrameTime = 1.0 / 75.0;
    double lastTime = 0;
    double deltaTime = 0;
    while (!glfwWindowShouldClose(window) && !done)
    {
        double frameTime = glfwGetTime();
        deltaTime = frameTime - lastTime;
        if (deltaTime < targetFrameTime)
        {
            continue;
        }

        Vec2 shakeOffset = handleShake(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);


        for (auto& s : shapes) {
            glBindVertexArray(s.vao);

            glUniform1f(glGetUniformLocation(shader, "u_rotation"), s.rotation);
            glUniform1f(glGetUniformLocation(shader, "u_scale"), s.scale);
            glUniform2f(glGetUniformLocation(shader, "u_shake_offset"), shakeOffset.x, shakeOffset.y);
            glUniform1f(glGetUniformLocation(shader, "u_aspect_ratio"), aspectRatio);
            glUniform2f(glGetUniformLocation(shader, "u_position"), s.position.x, s.position.y);

            glDrawArrays(GL_TRIANGLE_FAN, 0, s.vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        lastTime = glfwGetTime();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}