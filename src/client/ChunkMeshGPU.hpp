#pragma once

#include <glm/glm.hpp>

#include "../common/chunk.hpp"
#include "chunk_mesh_gen.hpp"

#include "render/IndexBuffer.hpp"
#include "render/VertexBuffer.hpp"
#include "render/VertexBufferLayout.hpp"
#include "render/shader.hpp"
#include "render/Texture.hpp"

#include "../common/vectors.hpp"

class Renderer;

class ChunkMeshGPU
{
    struct Buffers
    {
        VertexBuffer vb;
        int height;
        size_t vert_count;
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
    void Draw(Mat4x4 mvp,Chunk& c,Vector2Int pos_,Renderer& r);
    static void staticInit();

    inline ChunkMeshGPU(Chunk& c)
    {
        auto mesh = c.GenMesh();
        pos = c.getPos();
        Construct(mesh);
    }

    inline static void Draw(Chunk& c,Mat4x4 mvp,Vector2Int pos_,Renderer& r)
    {
        ChunkMeshGPU*& mesh = c.GPUMesh;
        if(mesh)
            mesh->Draw(mvp,c,pos_,r);
        else
        {
            mesh = new ChunkMeshGPU(c);
            mesh->Draw(mvp,c,pos_,r);
        }
    }
    
    inline ChunkMeshGPU(ChunkMesh& m)
    {
        Construct(m);
    }
};
