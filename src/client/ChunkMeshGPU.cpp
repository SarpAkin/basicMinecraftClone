#include "ChunkMeshGPU.h"

#include "../common/utility.h"

VertexBufferLayout ChunkMeshGPU::vlayout;
Shader ChunkMeshGPU::chunkShader;
Texture ChunkMeshGPU::atlas;

void ChunkMeshGPU::Construct(ChunkMesh& mesh)
{

    for (int i = 0;i < vertical_chunk_count;++i)
    {
        if (mesh.meshes[i].verticies.size())
        {
            buffers.push_back({
                IndexBuffer(mesh.meshes[i].indicies),
                VertexBuffer(mesh.meshes[i].verticies,vlayout),i
                });
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

void ChunkMeshGPU::Draw(Mat4x4 mvp,Vector2Int pos_)
{
    chunkShader.Bind();
    chunkShader.SetUniformMat4("u_MVP", mvp);
    for (auto& b : buffers)
    {
        chunkShader.SetUniformMat4("u_MVP", mvp * glm::translate(Mat4x4(1.0f), Vector3(pos_.x * chunk_size, b.height * chunk_size, pos_.y * chunk_size)));
        b.vb.Bind();
        b.ib.Bind();
        GLCALL(glDrawElements(GL_TRIANGLES, b.ib.size / sizeof(uint16_t), GL_UNSIGNED_SHORT, nullptr));
    }
}
