#pragma once

#include <glm/glm.hpp>

#include "chunk.h"

#include "render/IndexBuffer.h"
#include "render/VertexBuffer.h"
#include "render/VertexBufferLayout.h"
#include "render/shader.h"
#include "render/Texture.h"

#include "vectors.h"


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
    void Draw(Mat4x4 mvp);
    static void staticInit();

    inline ChunkMeshGPU(Chunk& c)
    {
        auto mesh = c.GenMesh();
        pos = c.getPos();
        Construct(mesh);
    }
    inline ChunkMeshGPU(ChunkMesh& m)
    {
        Construct(m);
    }
};
