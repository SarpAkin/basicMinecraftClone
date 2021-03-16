#include "ChunkMeshGPU.hpp"

#include "render/renderer.hpp"

#include "../common/utility.hpp"
#include <glm/ext/matrix_transform.hpp>

VertexBufferLayout ChunkMeshGPU::vlayout;
Shader ChunkMeshGPU::chunkShader;
Texture ChunkMeshGPU::atlas;

void ChunkMeshGPU::Construct(ChunkMesh& mesh)
{

    for (int i = 0; i < vertical_chunk_count; ++i)
    {
        if (mesh.meshes[i].verticies.size())
        {
            buffers.push_back(
                {IndexBuffer(mesh.meshes[i].indicies), VertexBuffer(mesh.meshes[i].verticies, vlayout), i});
        }
    }
}

void ChunkMeshGPU::staticInit()
{
    chunkShader = Shader(readFile("res/shaders/vertex.glsl"), readFile("res/shaders/fragment.glsl"));

    vlayout.Push<float>(3);
    vlayout.Push<float>(2);

    atlas = Texture("res/textures/tileatlas.png");
    atlas.Bind(0);

    chunkShader.Bind();
    chunkShader.SetUniform1i("u_Texture", 0);
}

void ChunkMeshGPU::Draw(Mat4x4 mvp, Chunk& c, Vector2Int pos_, Renderer& r)
{
    chunkShader.Bind();
    chunkShader.SetUniformMat4("u_MVP", mvp);
    for (auto& b : buffers)
    {
        auto c_mvp = mvp * glm::translate(Mat4x4(1.0f),
                               Vector3(pos_.x * chunk_size, b.height * chunk_size, pos_.y * chunk_size));
        chunkShader.SetUniformMat4("u_MVP", c_mvp);
        b.vb.Bind();
        b.ib.Bind();
        GLCALL(glDrawElements(GL_TRIANGLES, b.ib.size / sizeof(uint16_t), GL_UNSIGNED_SHORT, nullptr));
    }
    auto c_mvp = mvp * glm::translate(Mat4x4(1.0f), Vector3(pos_.x * chunk_size, 0, pos_.y * chunk_size));
    for (auto& e : c.Entities)
    {
        if(e->isVisible)
        e->Draw(c_mvp * glm::translate(glm::mat4x4(1.0f), e->transform.pos), r);
    }
}
