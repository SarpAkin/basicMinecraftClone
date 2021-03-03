#include <chrono>

#include "renderer.h"

std::array<uint8_t, KeyCount> Renderer::keyMap;
double Renderer::MouseXRaw;
double Renderer::MouseYRaw;

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
    if (key < KeyCount &&key >= 0)
        keyMap[key] = action;
    //std::cout << action << ' ' << key << '\n';
}

double oldxpos = 0;
double oldypos = 0;
void Renderer::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    MouseXRaw = xpos - oldxpos;
    MouseYRaw = ypos - oldypos;
    oldxpos = xpos;
    oldypos = ypos;
}

bool Renderer::Construct(int width_, int height_)
{
    width = width_;
    height = height_;

    /* Initialize the library */
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
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

    //Get keyboard-mouse input
    GLCALL(glfwSetKeyCallback(window, key_callback));
    GLCALL(glfwSetCursorPosCallback(window, cursor_position_callback));

    //GLCALL(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    if (glfwRawMouseMotionSupported())
    {
        GLCALL(glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE));
    }
    OnStart();

    std::cout << glGetString(GL_VERSION) << '\n';
    return true;
}

bool Renderer::Start()
{
    isRunning = true;
    auto s = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window) && isRunning)
    {
        auto e = s;
        s = std::chrono::steady_clock::now();
        auto dur = s - e;
        uint64_t dur_ = dur.count();

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