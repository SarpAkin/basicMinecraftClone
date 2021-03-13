#pragma once

#include <glm/glm.hpp>

#include "../common/chunk.hpp"

#include "render/IndexBuffer.hpp"
#include "render/VertexBuffer.hpp"
#include "render/VertexBufferLayout.hpp"
#include "render/shader.hpp"
#include "render/Texture.hpp"

#include "../common/vectors.hpp"


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
    static Shader chunkShader;
    static VertexBufferLayout vlayout;
    static Texture atlas;
private:
    void Construct(ChunkMesh& mesh);
public:
    ChunkMeshGPU() = default;
    void Draw(Mat4x4 mvp,Vector2Int pos_);
    static void staticInit();

    inline ChunkMeshGPU(Chunk& c)
    {
        auto mesh = c.GenMesh();
        pos = c.getPos();
        Construct(mesh);
    }

    inline static void Draw(Chunk& c,Mat4x4 mvp,Vector2Int pos_)
    {
        ChunkMeshGPU*& mesh = c.GPUMesh;
        if(mesh)
            mesh->Draw(mvp,pos_);
        else
        {
            mesh = new ChunkMeshGPU(c);
            mesh->Draw(mvp,pos_);
        }
    }
    
    inline ChunkMeshGPU(ChunkMesh& m)
    {
        Construct(m);
    }
};
