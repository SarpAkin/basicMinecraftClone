#include "ChunkMeshGPU.hpp"

#include "chunk_mesh_gen.hpp"
#include "render/renderer.hpp"

#include "../common/utility.hpp"

#include <glm/ext/matrix_transform.hpp>

VertexBufferLayout ChunkMeshGPU::vlayout;
Shader ChunkMeshGPU::chunkShader;
Shader ChunkMeshGPU::chunkShaderG;
Texture ChunkMeshGPU::atlas;
IndexBuffer ChunkMeshGPU::shared_ib;

void ChunkMeshGPU::Construct(ChunkMesh& meshes)
{

    for (int i = 0; i < vertical_chunk_count; ++i)
    {
        if (meshes[i].size())
        {
            buffers.push_back({VertexBuffer(meshes[i], vlayout), i, meshes[i].size()});
        }
    }
}

void ChunkMeshGPU::staticInit()
{
    chunkShader /* */ = Shader(readFile("res/shaders/chunk.vert"), readFile("res/shaders/chunk.frag"));
    chunkShaderG /**/ = Shader(readFile("res/shaders/chunk_g.vert"), readFile("res/shaders/chunk_g.frag"));

    vlayout.Push<uint32_t>(1);

    bind_atlas();

    std::vector<uint16_t> indicies;
    indicies.reserve(max_quad_size * 6);

    for (uint16_t i = 0; i < max_quad_size; ++i)
    {
        int vert_offset = i * 4;

        uint16_t arr[]{
            0, 1, 2, //
            2, 1, 3  //
        };

        for (size_t j = 0; j < 6; ++j)
            arr[j] += vert_offset;

        indicies.insert(indicies.end(), arr, arr + 6);
    }

    shared_ib = IndexBuffer(indicies);
}

void ChunkMeshGPU::bind_atlas()
{
    atlas = Texture("res/textures/tileatlas.png");
    atlas.Bind(atlas_slot);

    chunkShader.Bind();
    chunkShader.SetUniform1i("u_Texture", atlas_slot);

    chunkShaderG.Bind();
    chunkShaderG.SetUniform1i("u_Texture", atlas_slot);
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
        shared_ib.Bind();
        if (b.quad_count <= max_quad_size)
        {
            GLCALL(glDrawElements(GL_TRIANGLES, b.quad_count * 6, GL_UNSIGNED_SHORT, nullptr));
        }
    }
    auto c_mvp = mvp * glm::translate(Mat4x4(1.0f), Vector3(pos_.x * chunk_size, 0, pos_.y * chunk_size));
    for (auto& e : c.Entities)
    {
        if (e->isVisible)
            e->Draw(c_mvp * glm::translate(glm::mat4x4(1.0f), e->transform.pos), r);
    }
}

void ChunkMeshGPU::DrawG(Mat4x4 mvp, Chunk& c, Vector2Int pos_, Renderer& r)
{
    chunkShaderG.Bind();
    chunkShaderG.SetUniformMat4("u_MVP", mvp);


    for (auto& b : buffers)
    {
        auto c_pos = Vector3(pos_.x * chunk_size, b.height * chunk_size, pos_.y * chunk_size);

        auto c_mvp = mvp * glm::translate(Mat4x4(1.0f), c_pos);
        chunkShaderG.SetUniformMat4("u_MVP", c_mvp);
        chunkShaderG.SetUniform3f("u_chunk_pos", c_pos);
        b.vb.Bind();
        shared_ib.Bind();
        if (b.quad_count <= max_quad_size)
        {
            GLCALL(glDrawElements(GL_TRIANGLES, b.quad_count * 6, GL_UNSIGNED_SHORT, nullptr));
        }
    }
    auto c_mvp = mvp * glm::translate(Mat4x4(1.0f), Vector3(pos_.x * chunk_size, 0, pos_.y * chunk_size));
    for (auto& e : c.Entities)
    {
        if (e->isVisible)
            e->Draw(c_mvp * glm::translate(glm::mat4x4(1.0f), e->transform.pos), r);
    }
}