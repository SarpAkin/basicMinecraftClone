#pragma once

#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "opengl_.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "shader.h"

class Renderer
{
private:
    GLFWwindow* window;
    bool isRunning = false;

protected:
    virtual void OnStart();
    virtual void OnUpdate(double DeltaT);
    virtual void OnDestroy();
public:
    Renderer() = default;
    virtual ~Renderer();

    bool Construct(int width = 640,int height = 480);
    bool Start();

    void Draw(const VertexArray& va,const IndexBuffer& ib,const Shader& shader);
    void DrawU16(const VertexArray& va,const IndexBuffer& ib,const Shader& shader);
};

class renderer_
{
private:
    std::unique_ptr<Renderer> renderer;
public:
    renderer_(std::unique_ptr<Renderer> r_);
    ~renderer_();

    inline Renderer& getRenderer()
    {
        return *renderer;
    }
};

