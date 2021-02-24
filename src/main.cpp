#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#include <glm/gtc/matrix_transform.hpp>


#include "render/opengl_.h"
#include "render/IndexBuffer.h"
#include "render/VertexBuffer.h"
#include "render/VertexArray.h"
#include "render/VertexBufferLayout.h"
#include "render/shader.h"
#include "render/renderer.h"
#include "render/Texture.h"

#include "chunk.h"

std::string readFile(const char* file);

inline std::string readFile(const std::string& file)
{
    return readFile(file.c_str());
}

class ChunkMeshGPU
{
    struct Buffers
    {
        IndexBuffer ib;
        VertexBuffer vb;
        int height;
    };
    std::vector<Buffers> buffers;
    Vector2Int pos;
    VertexArray va = VertexArray(0);
    static Shader chunkShader;
    static VertexBufferLayout vlayout;
    static Texture atlas;
public:
    static void staticInit()
    {
        chunkShader = Shader(readFile("res/shaders/vertex.glsl"), readFile("res/shaders/fragment.glsl"));

        vlayout.Push<float>(3);
        vlayout.Push<float>(2);

        atlas = Texture("res/textures/tileatlas.png");
        atlas.Bind(0);

        chunkShader.Bind();
        chunkShader.SetUniform1i("u_Texture", 0);
    }
    ChunkMeshGPU() = default;
    ChunkMeshGPU(Chunk& c)
    {
        va = VertexArray();
        auto mesh = c.GenMesh();
        pos = c.getPos();

        for (int i = 0;i < vertical_chunk_count;++i)
        {
            if (mesh.meshes[i].verticies.size())
            {
                buffers.push_back({
                    IndexBuffer(mesh.meshes[i].indicies),
                    VertexBuffer(mesh.meshes[i].verticies),i
                    });
                va.AddBuffer(buffers.back().vb, vlayout);
            }
        }
    }
    ChunkMeshGPU(ChunkMesh& mesh)
    {
        va = VertexArray();
        pos = Vector2Int(0, 0);

        for (int i = 0;i < vertical_chunk_count;++i)
        {
            if (mesh.meshes[i].verticies.size())
            {
                buffers.push_back({
                    IndexBuffer(mesh.meshes[i].indicies),
                    VertexBuffer(mesh.meshes[i].verticies),i
                    });
                va.AddBuffer(buffers.back().vb, vlayout);
            }
        }
    }
    void Draw(Mat4x4 mvp)
    {
        va.Bind();
        chunkShader.Bind();
        chunkShader.SetUniformMat4("u_MVP", mvp);
        for (auto& b : buffers)
        {
            chunkShader.SetUniformMat4("u_MVP", mvp * glm::translate(Mat4x4(1), Vector3(pos.x * chunk_size, b.height * chunk_size, pos.y * chunk_size)));
            b.vb.Bind();
            b.ib.Bind();
            GLCALL(glDrawElements(GL_TRIANGLES, b.ib.size / sizeof(uint16_t), GL_UNSIGNED_SHORT, nullptr));
        }
    }
};
VertexBufferLayout ChunkMeshGPU::vlayout;
Shader ChunkMeshGPU::chunkShader;
Texture ChunkMeshGPU::atlas;

class TestRen : public Renderer
{


    ChunkMeshGPU mesh;
    glm::mat4 proj;
    glm::mat4 view;

public:
    TestRen()
    {

    }
    void OnStart() override
    {
        proj = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        view = glm::lookAt(
            glm::vec3(0, 10, 0), // Camera is at (4,3,3), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        Chunk c;
        for (int x = 0; x < 16; ++x)
        {
            for (int z = 0;z < 16;++z)
            {
                c[{x, 0, z}] = Tile(stone);
            }
        }
        ChunkMeshGPU::staticInit();
        mesh = ChunkMeshGPU(c);

    }
    float x_cord = 0;
    int sign = 1;
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
            glm::vec3(x_cord + 8, 3 - x_cord, -x_cord/2 + 8), // Camera is at (4,3,3), in World Space
            glm::vec3(0, -1, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        mesh.Draw(proj * view);
    }
};


int main()
{
    TestRen t = TestRen();
    if (t.Construct())
        t.Start();

}