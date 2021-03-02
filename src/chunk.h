#pragma once

#include <unordered_map>
#include <inttypes.h>
#include <memory>
#include <array>

#include "vectors.h"
#include "hasher.h"
#include "tile.h"



const int chunk_size = 16;
const int chunk_area = chunk_size * chunk_size;
const int chunk_volume = chunk_area * chunk_size;

const int vertical_chunk_count = 4;
const int max_block_height = chunk_size * vertical_chunk_count;

enum class direction : uint8_t
{
    up, down,
    north, south,
    west, east
};

struct ChunkVertex
{
    Vector3 pos;
    Vector2 textpos;
    inline ChunkVertex()
    {
        pos = { 0,0,0 };
        textpos = { 0,0 };
    }
    inline ChunkVertex(Vector3 pos_, Vector2 textpos_)
    {
        pos = pos_;textpos = textpos_;
    }
};

struct VerticalChunkMesh
{
    std::vector<ChunkVertex> verticies = std::vector<ChunkVertex>(0);
    std::vector<uint16_t> indicies = std::vector<uint16_t>(0);
    void addSquare(Vector3Int pos, direction facing, uint16_t textureID);

    inline void reserverSquares(size_t amount)
    {
        verticies.reserve(amount * 4);
        indicies.reserve(amount * 6);
    }
};

struct ChunkMesh
{
    std::array<VerticalChunkMesh, 4> meshes;
};

class ChunkMeshGPU;

class Chunk
{

    class TileRef
    {
        friend Chunk;
        Chunk& chunk;
        Vector3Int pos;
        inline TileRef(Chunk& c, Vector3Int p)
            :chunk(c)
        {
            pos = p;
        }

    public:
        void operator=(Tile);
        operator Tile() const;
    };

private:

    std::array<std::unique_ptr<std::array<Tile, chunk_volume>>, vertical_chunk_count> grid;
public:
    Chunk* northernChunk = nullptr; //z+
    Chunk* southernChunk = nullptr; //z-
    Chunk* easternChunk = nullptr; //x+
    Chunk* westernChunk = nullptr; //x-
//public:
    Vector2Int pos;
    ChunkMeshGPU* GPUMesh = nullptr;
private:
    inline void exc()
    {
        throw "chunk doesn't exist!";
    }


public:
    /*use [] operator instead of this!*/
    TileRef findBlockInChunk(Vector3Int);
    /*use [] operator instead of this!*/
    Tile findBlockInChunk(Vector3Int) const;
    Chunk() = default;
    inline Chunk(Vector2Int pos)
    {
        this->pos = pos;
    }
    ~Chunk();

    inline Vector2Int getPos()
    {
        return pos;
    }
    void Init(std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>>& Chunks);
    ChunkMesh GenMesh() const;

    Tile operator[] (Vector3Int pos) const;
    TileRef operator[] (Vector3Int pos);
};
