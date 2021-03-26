#include <chrono>
#include <unordered_map>

#include "renderer.hpp"

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)
{
    va.Bind();
    ib.Bind();
    shader.Bind();
    GLCALL(glDrawElements(GL_TRIANGLES, ib.size / sizeof(uint32_t), GL_UNSIGNED_INT, nullptr));
}

void Renderer::DrawU16(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)
{
    va.Bind();
    ib.Bind();
    shader.Bind();
    GLCALL(glDrawElements(GL_TRIANGLES, ib.size / sizeof(uint16_t), GL_UNSIGNED_SHORT, nullptr));
}

void Renderer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    if (key < KeyCount && key >= 0)
        renderer->keyMap[key] = action;

    if (action == GLFW_PRESS)
    {
        auto funcit = renderer->OnKey_Press_Funcs.find(key);
        if (funcit != renderer->OnKey_Press_Funcs.end())
        {
            funcit->second();
        }
    }
}

void Renderer::Screen_Resize_Callback(GLFWwindow* window, int width_, int height_)
{
    // GLCALL(glfwSetWindowSize(window, width_, height_));
    GLCALL(glViewport(0, 0, width_, height_));
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    renderer->width = width_;
    renderer->height = height_;
    renderer->OnScreenResize();
}

void Renderer::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    if (renderer->isCursorLocked)
    {
        renderer->MouseXRaw = xpos - renderer->oldxpos;
        renderer->MouseYRaw = ypos - renderer->oldypos;
    }
    else
    {
        renderer->MouseXRaw = 0;
        renderer->MouseYRaw = 0;
    }
    renderer->oldxpos = xpos;
    renderer->oldypos = ypos;
}

void Renderer::Mouse_Button_CallBack(GLFWwindow* window, int button, int action, int mods)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    if (button < MouseButtonCount && button >= 0)
    {
        renderer->MBMap[button] = action;
    }
    if (action == GLFW_PRESS)
    {
        auto funcit = renderer->OnMB_Press_Funcs.find(button);
        if (funcit != renderer->OnMB_Press_Funcs.end())
        {
            funcit->second();
        }
    }
}

bool Renderer::Construct(int width_, int height_)
{
    width = width_;
    height = height_;

    /* Initialize the library */
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Minecraft Clone", nullptr, nullptr);

    GLCALL(glfwSetWindowUserPointer(window, this));

    if (!window)
    {
        GLCALL(glfwTerminate());
        return false;
    }

    /* Make the window's context current */
    GLCALL(glfwMakeContextCurrent(window));

    if (glewInit() != GLEW_OK)
    {
        std::cout << "glew failed to initialize. exiting\n";
        return false;
    }

    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Enable depth test
    GLCALL(glEnable(GL_DEPTH_TEST));
    // Accept fragment if it closer to the camera than the former one
    GLCALL(glDepthFunc(GL_LESS));

    // Get keyboard-mouse input
    GLCALL(glfwSetKeyCallback(window, key_callback));
    GLCALL(glfwSetCursorPosCallback(window, cursor_position_callback));
    GLCALL(glfwSetWindowSizeCallback(window, Screen_Resize_Callback));
    GLCALL(glfwSetMouseButtonCallback(window, Mouse_Button_CallBack));

    OnStart();

    // std::cout << glGetString(GL_VERSION) << '\n';
    return true;
}

bool Renderer::Start()
{
    isRunning = true;
    auto s = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window) && isRunning)
    {
        /*
        auto e = s;
        s = std::chrono::steady_clock::now();
        auto dur = s - e;
        uint64_t dur_ = dur.count();*/
        uint64_t dur_ =  1000000000 / 144;

        /* Render here */
        GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        OnUpdate((double)dur_ / 1000000000);
        MouseXRaw = 0;
        MouseYRaw = 0;

        /* Swap front and back buffers */
        GLCALL(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCALL(glfwPollEvents());
    }
    return true;
}

void Renderer::LockCursor()
{
    GLCALL(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    if (glfwRawMouseMotionSupported())
    {
        GLCALL(glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE));
    }
    isCursorLocked = true;
}

void Renderer::UnlockCursor()
{
    isCursorLocked = false;
    GLCALL(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL));
}

Renderer::~Renderer()
{
    OnDestroy();
}

void Renderer::OnStart()
{
    /*leave Empty*/
}
void Renderer::OnUpdate(double DeltaT)
{
    /*leave Empty*/
}
void Renderer::OnDestroy()
{
    /*leave Empty*/
}
