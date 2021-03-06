#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>


#include "../../common/vectors.hpp"

#include "opengl_.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"
#include "shader.hpp"

const int KeyCount = GLFW_KEY_LAST + 1;
const int MouseButtonCount = GLFW_MOUSE_BUTTON_LAST + 1;
class Renderer
{
private:
    GLFWwindow* window;
    bool isRunning = false;

    double oldxpos = 0;
    double oldypos = 0;

protected:
    int width;
    int height;

    bool isCursorLocked = false;

    std::unordered_map<int,std::function<void()>> OnMB_Press_Funcs;
    std::unordered_map<int,std::function<void()>> OnKey_Press_Funcs;

    Vector4 clear_color = {0.0f,0.0f,0.0f,1.0f};
public:
    double MouseXRaw;
    double MouseYRaw;
    std::array<uint8_t, KeyCount> keyMap;
    std::array<uint8_t, MouseButtonCount> MBMap;

private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void Screen_Resize_Callback(GLFWwindow* window, int width_, int height_);
    static void Mouse_Button_CallBack(GLFWwindow* window, int button, int action, int mods);

protected:
    virtual void OnStart();
    virtual void OnUpdate(double DeltaT);
    virtual void OnDestroy();

    void LockCursor();
    void UnlockCursor();
public:
    Renderer() = default;
    virtual ~Renderer();
    virtual void OnScreenResize() = 0;

    bool Construct(int width = 640, int height = 480);
    bool Start();

    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
    void DrawU16(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);

};
