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
                {VertexBuffer(mesh.meshes[i].verticies, vlayout), i,mesh.meshes[i].verticies.size()});
        }
    }
}

void ChunkMeshGPU::staticInit()
{
    chunkShader = Shader(readFile("res/shaders/vertex.glsl"), readFile("res/shaders/fragment.glsl"));

    vlayout.Push<uint32_t>(1);

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
        GLCALL(glDrawArrays(GL_TRIANGLES,0,b.vert_count));

        
    }
    auto c_mvp = mvp * glm::translate(Mat4x4(1.0f), Vector3(pos_.x * chunk_size, 0, pos_.y * chunk_size));
    for (auto& e : c.Entities)
    {
        if(e->isVisible)
        e->Draw(c_mvp * glm::translate(glm::mat4x4(1.0f), e->transform.pos), r);
    }
}
