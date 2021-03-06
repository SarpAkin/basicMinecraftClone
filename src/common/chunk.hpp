#pragma once

#include <array>
#include <assert.h>
#include <functional>
#include <inttypes.h>
#include <iostream>
#include <tuple>
#include <memory>
#include <unordered_map>

#include "Entity.hpp"
#include "hasher.hpp"
#include "tile.hpp"
#include "utility.hpp"
#include "vectors.hpp"

const int chunk_size = 32;
const int chunk_area = chunk_size * chunk_size;
const int chunk_volume = chunk_area * chunk_size;

const int vertical_chunk_count = 16;
const int max_block_height = chunk_size * vertical_chunk_count;

enum Direction : uint8_t
{
    up,
    down,
    north,
    south,
    east,
    west
};

class GreedyMesher;

struct ChunkVertex
{
    Vector3 pos;
    Vector2 textpos;
    inline ChunkVertex()
    {
        pos = {0, 0, 0};
        textpos = {0, 0};
    }
    inline ChunkVertex(Vector3 pos_, Vector2 textpos_)
    {
        pos = pos_;
        textpos = textpos_;
    }
};

struct VoxelRect;
using ChunkMesh = std::array<std::vector<VoxelRect>, vertical_chunk_count>;

class ChunkMeshGPU;
class TGen;
class Game;

struct ChunkGenData
{
    std::array<uint16_t, chunk_area> heightmap;
    std::array<uint8_t, chunk_area> biomeMap;
    std::vector<std::tuple<Vector3Int,Tile>> blocks_to_place;
};

class Chunk
{
    friend TGen;
    friend Game;
    friend GreedyMesher;

public:
    static std::array<Tile, chunk_volume> air_vertical_chunk;

    class TileRef
    {
        friend Chunk;

    public:
        const Vector3Int pos;
        Chunk& chunk;

    private:
        inline TileRef(Chunk& c, Vector3Int p) : pos(p), chunk(c)
        {
        }

    public:
        void operator=(Tile);
        operator Tile() const;
    };

private:
    std::array<std::unique_ptr<std::array<Tile, chunk_volume>>, vertical_chunk_count> grid;

    std::vector<Vector3Int> light_sources;
    bool cached_lighed_sources = false;

public:
    Chunk* northernChunk = nullptr; // z+
    Chunk* southernChunk = nullptr; // z-
    Chunk* easternChunk = nullptr;  // x+
    Chunk* westernChunk = nullptr;  // x-
    // public:
    Vector2Int pos;
    ChunkMeshGPU* GPUMesh = nullptr;
    std::unique_ptr<ChunkGenData> chunkGenData;
    std::vector<std::shared_ptr<Entity>> Entities;

private:
    inline void exc()
    {
        throw std::runtime_error("chunk doesn't exist!\n");
    }

public:
    GEN_SERIALIZATION_FUNCTIONS(pos, grid);

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

    std::vector<Vector3Int> search_tile(Tile t);

    inline void reset_light_sources()
    {
        light_sources = std::vector<Vector3Int>();
        cached_lighed_sources = false;
    }

    inline const std::vector<Vector3Int>& get_light_sources()
    {
        if (!cached_lighed_sources)
        {
            light_sources = search_tile(Tile::TileMap["glass"]);
            light_sources.shrink_to_fit();
            cached_lighed_sources = true;
        }

        return light_sources;
    }

    inline Vector2Int getPos()
    {
        return pos;
    }

    inline std::vector<std::shared_ptr<Entity>>::iterator GetEntityIt(std::shared_ptr<Entity> ent)
    {
        auto& chunkEnt = ent->currentChunk->Entities;
        for (auto it = chunkEnt.begin(); it != chunkEnt.end(); ++it)
            if (*it == ent)
            {
                return it;
            }
        return chunkEnt.end();
    }

    inline void MoveEntity(std::vector<std::shared_ptr<Entity>>::iterator e_it, Chunk& new_chunk)
    {
        (**e_it).currentChunk = &new_chunk;
        new_chunk.Entities.push_back(std::move(*e_it));
        Entities.erase(e_it);
    }

    void blockMeshUpdate(Vector3Int pos);
    void updateMesh();
    void resetNeighbour();
    void Tick(float deltaT);

    // positions should be relative to chunk pos
    bool RayCast(Vector3 start, Vector3 end, Vector3Int& hitTile, Vector3Int& facing);
    std::vector<Vector3> doesCollide(Transform& t);

    inline static Vector2Int ToChunkCord(Vector2Int in)
    {
        return Vector2Int(in.x >= 0 ? in.x / chunk_size : (in.x - (chunk_size - 1)) / chunk_size,
            in.y >= 0 ? in.y / chunk_size : (in.y - (chunk_size - 1)) / chunk_size);
    }
    inline static Vector2Int ToChunkCord(Vector3 in)
    {
        return Vector2Int(in.x >= 0 ? in.x / chunk_size : (std::ceil(in.x) / chunk_size) - 1,
            in.z >= 0 ? in.z / chunk_size : (std::ceil(in.z) / chunk_size) - 1);
    }

    void Init(std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>>& Chunks);
    ChunkMesh GenMesh() const;

    Tile operator[](Vector3Int pos) const;
    TileRef operator[](Vector3Int pos);
};

class ChunksInRange
{
    typedef std::function<void(Chunk& c, Vector2Int r_pos, Vector2Int f_pos)> func;
    func forEachChunk, ifDoesntExist;
    Vector2Int midPos;

    void East(Chunk* c, int range, Vector2Int pos);
    void West(Chunk* c, int range, Vector2Int pos);
    void South(Chunk* c, int range, Vector2Int pos);
    void North(Chunk* c, int range, Vector2Int pos);

public:
    ChunksInRange(Chunk* c, func forEachChunk_, func ifDoesntExist_, int range = 10);
};