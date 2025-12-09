#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>
#include <sstream>
#include <iomanip>

#include "../Header/Util.h"
#include "../Header/Shape.h"


std::vector<Shape> shapes;

float screenWidth;
float screenHeight;
float aspectRatio = 1.0f;
bool done = false;

bool isShaking;
double currentShakeDuration;
double maxShakeDuration = 0.2;
double shakeIntensity = 0.03;
double oscilation = 1;

double shakePeak = 0.3;
double shakeFalloff = 0.4;
unsigned int texCircle, texSquare, texTri, texRect, texEllipse, texHex;

struct TextCharacter {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

std::map<int, TextCharacter> Characters;

unsigned int textShader, textVAO, textVBO;
// Helper function to decode UTF-8 string to Unicode code points
std::vector<unsigned int> decodeUTF8(const std::string& utf8) {
    std::vector<unsigned int> unicode;

    for (size_t i = 0; i < utf8.size();) {
        unsigned char c = utf8[i];

        if (c <= 0x7F) {
            // ASCII character (1 byte)
            unicode.push_back(c);
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2-byte character
            if (i + 1 < utf8.size()) {
                unsigned int code = ((c & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
                unicode.push_back(code);
                i += 2;
            }
            else {
                i++; // Skip invalid
            }
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3-byte character (Serbian chars like ć, š are here)
            if (i + 2 < utf8.size()) {
                unsigned int code = ((c & 0x0F) << 12) |
                    ((utf8[i + 1] & 0x3F) << 6) |
                    (utf8[i + 2] & 0x3F);
                unicode.push_back(code);
                i += 3;
            }
            else {
                i++; // Skip invalid
            }
        }
        else if ((c & 0xF8) == 0xF0) {
            // 4-byte character
            if (i + 3 < utf8.size()) {
                unsigned int code = ((c & 0x07) << 18) |
                    ((utf8[i + 1] & 0x3F) << 12) |
                    ((utf8[i + 2] & 0x3F) << 6) |
                    (utf8[i + 3] & 0x3F);
                unicode.push_back(code);
                i += 4;
            }
            else {
                i++; // Skip invalid
            }
        }
        else {
            i++; // Skip invalid byte
        }
    }

    return unicode;
}
// Function to initialize text rendering
void initTextRendering() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, "Resources/HandsetSansUI.otf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load extended character set including special Serbian/Croatian characters
    std::vector<unsigned int> charSet;

    // Load ASCII characters (0-127)
    for (unsigned int c = 0; c < 128; c++) {
        charSet.push_back(c);
    }

    // Load extended Latin characters (including ć, š, č, đ, ž, etc.)
    // Note: These are Unicode code points, not extended ASCII
    charSet.push_back(0x0107); // ć - LATIN SMALL LETTER C WITH ACUTE
    charSet.push_back(0x0161); // š - LATIN SMALL LETTER S WITH CARON
    charSet.push_back(0x010D); // č - LATIN SMALL LETTER C WITH CARON
    charSet.push_back(0x0111); // đ - LATIN SMALL LETTER D WITH STROKE
    charSet.push_back(0x017E); // ž - LATIN SMALL LETTER Z WITH CARON
    charSet.push_back(0x0106); // Ć - LATIN CAPITAL LETTER C WITH ACUTE
    charSet.push_back(0x0160); // Š - LATIN CAPITAL LETTER S WITH CARON
    charSet.push_back(0x010C); // Č - LATIN CAPITAL LETTER C WITH CARON
    charSet.push_back(0x0110); // Đ - LATIN CAPITAL LETTER D WITH STROKE
    charSet.push_back(0x017D); // Ž - LATIN CAPITAL LETTER Z WITH CARON

    std::cout << "Loading font characters..." << std::endl;
    int loadedCount = 0;
    int failedCount = 0;

    for (unsigned int charCode : charSet) {
        if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
            failedCount++;
            std::cout << "Failed to load character: U+" << std::hex << charCode << std::dec << std::endl;
            continue; // Skip if character not found in font
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        TextCharacter character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<unsigned int, TextCharacter>(charCode, character));
        loadedCount++;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    std::cout << "Loaded " << loadedCount << " characters, failed: " << failedCount << std::endl;

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


// Function to render UTF-8 text
void renderText(unsigned int shader, const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniform2f(glGetUniformLocation(shader, "screenSize"), screenWidth, screenHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Decode UTF-8 string to Unicode code points
    std::vector<unsigned int> unicodeChars = decodeUTF8(text);

    for (unsigned int charCode : unicodeChars) {
        // Try to find the character in our map
        auto it = Characters.find(charCode);
        if (it == Characters.end()) {
            // Character not found - use a fallback (space or '?')
            // Check for '?' (ASCII 63) as fallback
            it = Characters.find(63); // ASCII code for '?'
            if (it == Characters.end()) {
                // Even '?' not found, skip this character but advance cursor
                x += 10 * scale; // Approximate width
                continue;
            }
        }

        TextCharacter ch = it->second;

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos - h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos - h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos - h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
auto fmt = [](float v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << v;
    return ss.str();
    };
void handleTextRendering()
{
    std::string durationText = "Shake Duration: " + fmt(maxShakeDuration);
    std::string intensityText = "Shake Intensity: " + fmt(shakeIntensity);
    std::string speedText = "Shake Oscilation: " + fmt(oscilation);

    renderText(textShader, "Uro\xC5\xA1 Raduki\xC4\x87", 10.0f, 40.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));    renderText(textShader, "SV54/2022", 10.0f, 70.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, durationText, 10.0f, 100.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, intensityText, 10.0f, 140.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, speedText, 10.0f, 180.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));

    renderText(textShader, "Controls:", 10.0f, 210.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "1/2: Decrease/Increase Duration", 10.0f, 235.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "3/4: Decrease/Increase Intensity", 10.0f, 260.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "5/6: Decrease/Increase Oscilation", 10.0f, 285.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "C,S,T,R,E,H: Add Shapes", 10.0f, 310.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "Space: Trigger Shake", 10.0f, 335.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(textShader, "ESC: Quit", 10.0f, 360.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
}


Vec2 screenToWorld(double mx, double my)
{
    float x = (float)mx / screenWidth * 2.0f - 1.0f;
    float y = 1.0f - (float)my / screenHeight * 2.0f;

    x *= aspectRatio;

    return { x, y };
}
Vec2 toLocal(Vec2 p, const Shape& s)
{
    // un-translate
    float x = p.x - s.position.x;
    float y = p.y - s.position.y;

    // un-rotate
    float cs = cos(-s.rotation);
    float sn = sin(-s.rotation);

    float xr = x * cs - y * sn;
    float yr = x * sn + y * cs;

    // un-scale
    xr /= s.scale;
    yr /= s.scale;

    return { xr, yr };
}

std::vector<Vec2> makeUVsForFan(std::vector<Vec2>& verts)
{
    std::vector<Vec2> uv;
    uv.reserve(verts.size());

    for (int i = 0; i < verts.size(); i++)
    {
        float u = (verts[i].x + 1.0f) * 0.5f;
        float v = (verts[i].y + 1.0f) * 0.5f;
        uv.push_back({ u, v });
    }
    return uv;
}

std::vector<Vec2> makeUVsForEllipseFan(const std::vector<Vec2>& verts)
{
    std::vector<Vec2> uv;
    uv.reserve(verts.size());

    float width = 2.0f;
    float height = 1.0f; 

    for (int i = 0; i < verts.size(); i++)
    {
        float u = (verts[i].x / width + 1.0f) * 0.5f;
        float v = (verts[i].y / height + 1.0f) * 0.5f;
        uv.push_back({ u, v });
    }

    return uv;
}


std::vector<Vec2> makeUVsForRect()
{
    return {
        {0,0},
        {1,0},
        {1,1},
        {0,1}
    };
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
        0,          
        2,          
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    s.vertexCount = verts.size() / 2;
    glBindVertexArray(0);

    return s;
}

void uploadShape(Shape& s, const std::vector<Vec2>& verts, const std::vector<Vec2>& uvs)
{
    std::vector<float> flat;
    flat.reserve(verts.size() * 4);

    for (int i = 0; i < verts.size(); i++)
    {
        flat.push_back(verts[i].x);
        flat.push_back(verts[i].y);
        flat.push_back(uvs[i].x);
        flat.push_back(uvs[i].y);
    }

    glGenVertexArrays(1, &s.vao);
    glBindVertexArray(s.vao);

    glGenBuffers(1, &s.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, flat.size() * sizeof(float), flat.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)(2 * sizeof(float))
    );

    s.vertexCount = verts.size();
}




void mouseClick(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        Vec2 pos = screenToWorld(mx, my);

        for (int i = (int)shapes.size() - 1; i >= 0; --i)   
        {
            Vec2 local = toLocal(pos, shapes[i]);
            if (pointInShape(local, shapes[i].baseVerts))
            {
                glDeleteBuffers(1, &shapes[i].vbo);
                glDeleteVertexArrays(1, &shapes[i].vao);
                shapes.erase(shapes.begin() + i);
                return;
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        Vec2 pos = screenToWorld(mx, my);

        Shape s;
        s.position = pos;
        float rotationDirection = rand() % 2;
        if (rotationDirection == 0)
            rotationDirection = -1;
        s.rotation = rotationDirection * (rand() % 30);
        s.scale = 0.05f + (rand() % 100) / 1000.0f;
        s.hasTexture = true;

        std::vector<Vec2> verts, uvs;

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            verts = makeCircle();
            s.texture = texCircle;
            uvs = makeUVsForFan(verts);
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            verts = makeSquare();
            s.texture = texSquare;  
            uvs = makeUVsForRect();
        }
        else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            verts = makeTriangle();
            s.texture = texTri;
            uvs = makeUVsForFan(verts);

        }
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            verts = makeRectangle();
            s.texture = texRect;
            uvs = makeUVsForRect();

        }
        else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            verts = makeEllipse();
            s.texture = texEllipse;
            uvs = makeUVsForEllipseFan(verts);

        }
        else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            verts = makeHexagon();
            s.texture = texHex;
            uvs = makeUVsForFan(verts);

        }
        else
            return;

        s.baseVerts = verts;
        uploadShape(s, verts, uvs);
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
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_1) maxShakeDuration -= 0.01f;
    if (key == GLFW_KEY_2) maxShakeDuration += 0.01f;

    if (key == GLFW_KEY_3) shakeIntensity -= 0.01f;
    if (key == GLFW_KEY_4) shakeIntensity += 0.01f;

    if (key == GLFW_KEY_5) oscilation -= 0.01f;
    if (key == GLFW_KEY_6) oscilation += 0.01f;

    if (maxShakeDuration < 0) maxShakeDuration = 0.00f;
    if (shakeIntensity < 0) shakeIntensity = 0.00f;
    if (oscilation < 0) oscilation = 0.00f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) done = true;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !isShaking)
    {
        isShaking = true;
        currentShakeDuration = 0;
        seed1 = rand() % 10000;
        seed2 = rand() % 10000;
        seed3 = rand() % 10000;
        seed4 = rand() % 10000;
        seed5 = rand() % 10000;
        seed6 = rand() % 10000;
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

    float bigShakeX = (float)cos((currentShakeDuration + seed1)* oscilation * 100) * shakeIntensity;
    float mediumShakeX = (float)cos((currentShakeDuration + seed2) * oscilation * 1000) * shakeIntensity/10;
    float smallShakeX = (float)cos((currentShakeDuration + seed3) * oscilation * 10000) * shakeIntensity/100;

    float totalShakeX = bigShakeX + mediumShakeX + smallShakeX; 

    float bigShakeY = (float)cos((currentShakeDuration + seed4) * oscilation * 100) * shakeIntensity;
    float mediumShakeY = (float)cos((currentShakeDuration + seed5) * oscilation * 1000) * shakeIntensity/10;
    float smallShakeY = (float)cos((currentShakeDuration + seed6) * oscilation * 10000) * shakeIntensity/100;

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
    GLFWwindow* window = glfwCreateWindow(
        screenWidth,
        screenHeight,
        "My Fullscreen Window",
        monitor,     
        nullptr      
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

    glClearColor(0.1f, 0.9f, 0.4f, 1.0f);

    unsigned int shader = createShader("Shaders/shake.vert", "Shaders/shake.frag");
    glUseProgram(shader);

    preprocessTexture(texCircle, "Resources/circle_house.png");
    preprocessTexture(texEllipse, "Resources/elipse_house.png");
    preprocessTexture(texTri, "Resources/triangle_house.png");
    preprocessTexture(texHex, "Resources/hexagon_house.png");
    preprocessTexture(texSquare, "Resources/square_house.png");
    preprocessTexture(texRect, "Resources/rectangle_house.png");

    textShader = createShader("Shaders/text.vert", "Shaders/text.frag");

    initTextRendering();

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
            glUseProgram(shader);
            glBindVertexArray(s.vao);

            if (s.hasTexture)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, s.texture);
                glUniform1i(glGetUniformLocation(shader, "u_tex"), 0);
            }
            
            glUniform1i(glGetUniformLocation(shader, "u_hasTexture"), s.hasTexture ? 1 : 0);

            glUniform1f(glGetUniformLocation(shader, "u_rotation"), s.rotation);
            glUniform1f(glGetUniformLocation(shader, "u_scale"), s.scale);
            glUniform2f(glGetUniformLocation(shader, "u_shake_offset"), shakeOffset.x, shakeOffset.y);
            glUniform1f(glGetUniformLocation(shader, "u_aspect_ratio"), aspectRatio);
            glUniform2f(glGetUniformLocation(shader, "u_position"), s.position.x, s.position.y);

            glDrawArrays(GL_TRIANGLE_FAN, 0, s.vertexCount);
        }
        handleTextRendering();
        glUseProgram(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
        lastTime = glfwGetTime();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}