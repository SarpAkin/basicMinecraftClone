#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#include <glm/gtc/matrix_transform.hpp>


#include "opengl_.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "shader.h"
#include "renderer.h"
#include "Texture.h"

/*
struct Vector2
{
    float x;
    float y;
    inline Vector2(float x_, float y_)
    {
        x = x_;
        y = y_;
    }
};
*/


typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;

std::string readFile(const char* file)
{
    std::fstream stream;
    stream.open(file, std::ios::in);
    if (stream.is_open())
        return std::string(std::istreambuf_iterator<char>(stream), {});
    else
        return std::string();
}

inline std::string readFile(const std::string& file)
{
    return readFile(file.c_str());
}

struct Vertex
{
    Vector3 pos;
    Vector2 textpos;
} v = { Vector3(0,0,0),Vector2(0,0) };




class TestRen : public Renderer
{

    VertexBuffer vb;
    IndexBuffer ib;
    Shader shader;
    VertexArray va = VertexArray(0);
    Texture texture;

    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    void OnStart() override
    {

        //proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
        proj = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        view = glm::lookAt(
            glm::vec3(-6, 2, 0), // Camera is at (4,3,3), in World Space
            glm::vec3(1, 1, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        Vertex vertexes[] =
        {
            {Vector3(-0.5f, -0.5f, -0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(0.5f, -0.5f, -0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(0.5f,  0.5f, -0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(-0.5f,  0.5f, -0.5f),   Vector2(0.0f, 1.0f)},

            {Vector3(-0.5f, -0.5f,  0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(0.5f, -0.5f,  0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(0.5f,  0.5f,  0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(-0.5f,  0.5f,  0.5f),   Vector2(0.0f, 1.0f)},

            {Vector3(-0.5f,  0.5f, -0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(0.5f,  0.5f, -0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(0.5f,  0.5f,  0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(-0.5f,  0.5f,  0.5f),   Vector2(0.0f, 1.0f)},

            {Vector3(-0.5f, -0.5f, -0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(0.5f, -0.5f, -0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(0.5f, -0.5f,  0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(-0.5f, -0.5f,  0.5f),   Vector2(0.0f, 1.0f)},

            {Vector3(-0.5f, -0.5f, -0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(-0.5f, -0.5f,  0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(-0.5f,  0.5f,  0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(-0.5f,  0.5f, -0.5f),   Vector2(0.0f, 1.0f)},

            {Vector3(0.5f, -0.5f, -0.5f),   Vector2(0.0f, 0.0f)},
            {Vector3(0.5f, -0.5f,  0.5f),   Vector2(1.0f, 0.0f)},
            {Vector3(0.5f,  0.5f,  0.5f),   Vector2(1.0f, 1.0f)},
            {Vector3(0.5f,  0.5f, -0.5f),   Vector2(0.0f, 1.0f)}
        };

        uint32_t indexes[] =
        {
            0,1,2,
            0,2,3,

            4,5,6,
            4,6,7,

            8,9,10,
            8,10,11,

            12,13,14,
            12,14,15,

            16,17,18,
            16,18,19,

            20,21,22,
            20,22,23
        };

        vb = VertexBuffer (&vertexes, sizeof(vertexes));
        ib = IndexBuffer(&indexes, sizeof(indexes));

        VertexBufferLayout vlayout;
        vlayout.Push<float>(3);
        vlayout.Push<float>(2);

        va = VertexArray();
        va.AddBuffer(vb, vlayout);

        texture = Texture("maze.png");
        texture.Bind(0);

        shader = Shader(readFile("src/shaders/vertex.glsl"), readFile("src/shaders/fragment.glsl"));
        shader.Bind();
        //shader->SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);
        shader.SetUniform1i("u_Texture", 0);
    }

    float x_cord = 0;
    float sign = 1;
    void OnUpdate(double DeltaT) override
    {
        x_cord += (float)DeltaT * sign;
        if (x_cord > 5)
        {
            sign = -sign;
        }
        else if (x_cord < -5)
        {
            sign = -sign;
        }
        view = glm::lookAt(
            glm::vec3(1 + (x_cord / 1.5f), 3, x_cord), // Camera is at (4,3,3), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        glm::mat4 mvp;

        mvp = proj * view * glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0));
        shader.SetUniformMat4("u_MVP", mvp);
        Draw(va, ib, shader);

        mvp = proj * view * glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        shader.SetUniformMat4("u_MVP", mvp);
        Draw(va, ib, shader);
    }
};

int main_()
{


    TestRen t;
    if (t.Construct())
        t.Start();
    else
        std::cout << "aaaaaaaa\n";
    return 0;
}