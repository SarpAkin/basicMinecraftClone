#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <array>

#include "opengl_.hpp"

#include "VertexBufferLayout.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "shader.hpp"


const int KeyCount = GLFW_KEY_LAST + 1;
class Renderer
{
private:
    GLFWwindow* window;
    bool isRunning = false;
protected:
    int width;
    int height;
public:
    static double MouseXRaw;
    static double MouseYRaw;
    static std::array<uint8_t,KeyCount> keyMap;
private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

protected:
    virtual void OnStart();
    virtual void OnUpdate(double DeltaT);
    virtual void OnDestroy();
public:
    Renderer() = default;
    virtual ~Renderer();

    bool Construct(int width = 640, int height = 480);
    bool Start();

    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
    void DrawU16(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
};


