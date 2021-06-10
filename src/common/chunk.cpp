#include "chunk.hpp"
#include "tile.hpp"
#include "vectors.hpp"

#include <cmath>
#include <glm/geometric.hpp>
#include <iostream>

#ifndef SERVER_SIDE
#include "../client/ChunkMeshGPU.hpp"
#endif

#include "Physics.hpp"

void Chunk::Tick(float deltaT)
{
    if (Entities.size())
        for (int i = Entities.size() - 1; i >= 0; --i)
        {
            ChunkVSAABB(Entities.begin() + i, deltaT);
        }
}

// square func
inline float sq(float in)
{
    return in * in;
}

std::vector<Vector3> Chunk::doesCollide(Transform& t)
{
    const auto& c = *this;
    Vector3 ChunkRealPos = Vector3(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
    Vector3 inChunkPos = t.pos;

    std::vector<Vector3> blockColliders;
    blockColliders.reserve((int)((t.size.x + 1) * (t.size.y + 1) * (t.size.z + 1)));
    for (int x = std::floor(inChunkPos.x), x_end = std::ceil(inChunkPos.x + t.size.x) + 1; x < x_end; ++x)
        for (int y = std::floor(inChunkPos.y), y_end = std::ceil(inChunkPos.y + t.size.y) + 1; y < y_end; ++y)
            for (int z = std::floor(inChunkPos.z), z_end = std::ceil(inChunkPos.z + t.size.z) + 1; z < z_end; ++z)
            {
                if (c[{x, y, z}] != air)
                {
                    blockColliders.emplace_back(x, y, z);
                }
            }

    return blockColliders;
}

bool Chunk::RayCast(Vector3 start, Vector3 end, Vector3Int& hitTile, Vector3Int& facing)
{

    start += Vector3(.5f, .5f, .5f); // add .5 to all axises since tiles borders range form .5 to -.5
    end += Vector3(.5f, .5f, .5f);   // add .5 to all axises since tiles borders range form .5 to -.5

    Vector3 dir = glm::normalize(end - start);

    // if an axis is 0 function stucks in an infinite loop
    if (dir.x == 0.0f)
        dir.x = 0.00001f;
    if (dir.y == 0.0f)
        dir.y = 0.00001f;
    if (dir.z == 0.0f)
        dir.z = 0.00001f;

    Vector3Int startTilePos = (Vector3Int)(start);

    // clang-format off
    Vector3 stepSize = {
        std::sqrt(1 + sq(dir.y / dir.x) + sq(dir.z / dir.x)), // step size for x
        std::sqrt(1 + sq(dir.x / dir.y) + sq(dir.z / dir.y)), // step size for y
        std::sqrt(1 + sq(dir.x / dir.z) + sq(dir.y / dir.z)), // step size for z
    };
    // clang-format on

    float rayLength_x;
    float rayLength_y;
    float rayLength_z;

    int x_dir;
    int y_dir;
    int z_dir;
    if (dir.x < 0)
    {
        x_dir = -1;
        rayLength_x = (start.x - float(startTilePos.x)) * stepSize.x;
    }
    else
    {
        x_dir = 1;
        rayLength_x = (float(startTilePos.x + 1) - start.x) * stepSize.x;
    }

    if (dir.y < 0)
    {
        y_dir = -1;
        rayLength_y = (start.y - float(startTilePos.y)) * stepSize.y;
    }
    else
    {
        y_dir = 1;
        rayLength_y = (float(startTilePos.y + 1) - start.y) * stepSize.y;
    }

    if (dir.z < 0)
    {
        z_dir = -1;
        rayLength_z = (start.z - float(startTilePos.z)) * stepSize.z;
    }
    else
    {
        z_dir = 1;
        rayLength_z = (float(startTilePos.z + 1) - start.z) * stepSize.z;
    }

    const auto& this_ = *this;

    Vector3Int currentTile = startTilePos;
    float distance = std::sqrt(sq(start.x - end.x) + sq(start.y - end.y) + sq(start.z - end.z));

    while (true)
    {
        Vector3Int preIncrement = currentTile;
        // find the shortest ray
        if (rayLength_x < rayLength_y)
        {
            if (rayLength_x < rayLength_z)
            {
                // x is the shortest
                if (rayLength_x > distance)
                {
                    return false;
                }
                rayLength_x += stepSize.x;
                currentTile.x += x_dir;
            }
            else
            {
                // z is the shortest
                if (rayLength_z > distance)
                {
                    return false;
                }
                rayLength_z += stepSize.z;
                currentTile.z += z_dir;
            }
        }
        else
        {
            if (rayLength_y < rayLength_z)
            {
                // y is the shortest
                if (rayLength_y > distance)
                {
                    return false;
                }
                rayLength_y += stepSize.y;
                currentTile.y += y_dir;
            }
            else
            {
                // z is the shortest
                if (rayLength_z > distance)
                {
                    return false;
                }
                rayLength_z += stepSize.z;
                currentTile.z += z_dir;
            }
        }
        if (this_[currentTile].properties().isSolid)
        {
            hitTile = currentTile;
            facing = preIncrement - currentTile;
            return true;
        }
    }
    return false;
}

Chunk::TileRef Chunk::operator[](Vector3Int pos)
{
    if (pos.x < 0)
    {
        pos.x += chunk_size;
        if (westernChunk)
            return (*westernChunk)[pos];
        else
            exc();
    }
    else if (pos.x >= chunk_size)
    {
        pos.x -= chunk_size;
        if (easternChunk)
            return (*easternChunk)[pos];
        else
            exc();
    }

    if (pos.z < 0)
    {
        pos.z += chunk_size;
        if (southernChunk)
            return (*southernChunk)[pos];
        else
            exc();
    }
    else if (pos.z >= chunk_size)
    {
        pos.z -= chunk_size;
        if (northernChunk)
            return (*northernChunk)[pos];
        else
            exc();
    }

    return findBlockInChunk(pos);
}

Tile Chunk::operator[](Vector3Int pos) const
{
    if (pos.x < 0)
    {
        pos.x += chunk_size;
        if (westernChunk)
            return (*(const Chunk*)westernChunk)[pos];
        else
            return Tile(TileTypes::air);
    }
    else if (pos.x >= chunk_size)
    {
        pos.x -= chunk_size;
        if (easternChunk)
            return (*(const Chunk*)easternChunk)[pos];
        else
            return Tile(TileTypes::air);
    }

    if (pos.z < 0)
    {
        pos.z += chunk_size;
        if (southernChunk)
            return (*(const Chunk*)southernChunk)[pos];
        else
            return Tile(TileTypes::air);
    }
    else if (pos.z >= chunk_size)
    {
        pos.z -= chunk_size;
        if (northernChunk)
            return (*(const Chunk*)northernChunk)[pos];
        else
            return Tile(TileTypes::air);
    }

    return findBlockInChunk(pos);
}

Chunk::TileRef Chunk::findBlockInChunk(Vector3Int pos)
{
    return Chunk::TileRef(*this, pos);
}

Tile Chunk::findBlockInChunk(Vector3Int pos) const
{

    if (pos.y >= 0 && pos.y < max_block_height)
    {
        int slice = pos.y / chunk_size;
        if (auto& s = grid[slice])
            return ((Tile*)s.get())[pos.x + ((pos.y % chunk_size) * chunk_size) + (pos.z * chunk_area)];
    }

    // return air if it can't find
    return Tile(TileTypes::air);
}

void Chunk::updateMesh()
{
#ifndef SERVER_SIDE
    delete GPUMesh;
    GPUMesh = nullptr;
#endif
}

void Chunk::blockMeshUpdate(Vector3Int pos)
{
    updateMesh();
    if (pos.z == 15 && northernChunk)
    {
        northernChunk->updateMesh();
    }
    else if (pos.z == 0 && southernChunk)
    {
        southernChunk->updateMesh();
    }
    if (pos.x == 0 && westernChunk)
    {
        westernChunk->updateMesh();
    }
    else if (pos.x == 15 && easternChunk)
    {
        easternChunk->updateMesh();
    }
}

// TODO optimze this
ChunkMesh Chunk::GenMesh() const
{
    ChunkMesh mesh_;
    auto& this_ = (*this);
    for (int i = 0; i < vertical_chunk_count; ++i)
    {
        VerticalChunkMesh& mesh = mesh_.meshes[i];
        if (grid[i] != nullptr)
        {
            mesh = VerticalChunkMesh();
            mesh.reserverSquares(10000);
            for (int y = 0; y < chunk_size; ++y)
            {
                int globalY = y + i * chunk_size;
                for (int x = 0; x < chunk_size; ++x)
                {
                    for (int z = 0; z < chunk_size; ++z)
                    {
                        auto tile = this_[{x, globalY, z}];
                        auto textureID = tile.properties().TextureID;
                        if (tile.ID)
                        {
                            if (this_[{x, globalY + 1, z}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::up, textureID);

                            if (this_[{x, globalY - 1, z}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::down, textureID);

                            if (this_[{x, globalY, z + 1}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::north, textureID);

                            if (this_[{x, globalY, z - 1}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::south, textureID);

                            if (this_[{x + 1, globalY, z}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::east, textureID);

                            if (this_[{x - 1, globalY, z}].properties().isTransparent)
                                mesh.addSquare({x, y, z}, direction::west, textureID);
                        }
                    }
                }
            }
        }
    }
    return mesh_;
}

void Chunk::resetNeighbour()
{
    if (northernChunk)
    {
        northernChunk->southernChunk = nullptr;
    }
    if (southernChunk)
    {
        southernChunk->northernChunk = nullptr;
    }
    if (westernChunk)
    {
        westernChunk->easternChunk = nullptr;
    }
    if (easternChunk)
    {
        easternChunk->westernChunk = nullptr;
    }
}

Chunk::~Chunk()
{
#ifndef SERVER_SIDE
    delete GPUMesh;
#endif
    if (northernChunk)
    {
        northernChunk->southernChunk = nullptr;
        northernChunk->updateMesh();
    }
    if (southernChunk)
    {
        southernChunk->northernChunk = nullptr;
        southernChunk->updateMesh();
    }
    if (westernChunk)
    {
        westernChunk->easternChunk = nullptr;
        westernChunk->updateMesh();
    }
    if (easternChunk)
    {
        easternChunk->westernChunk = nullptr;
        easternChunk->updateMesh();
    }
}

void Chunk::Init(std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>>& Chunks)
{

    northernChunk = Chunks[pos + Vector2Int(0, 1)].get();
    southernChunk = Chunks[pos + Vector2Int(0, -1)].get();
    easternChunk = Chunks[pos + Vector2Int(1, 0)].get();
    westernChunk = Chunks[pos + Vector2Int(-1, 0)].get();

    if (northernChunk)
    {
        northernChunk->southernChunk = this;
        northernChunk->updateMesh();
    }
    if (southernChunk)
    {
        southernChunk->northernChunk = this;
        southernChunk->updateMesh();
    }
    if (westernChunk)
    {
        westernChunk->easternChunk = this;
        westernChunk->updateMesh();
    }
    if (easternChunk)
    {
        easternChunk->westernChunk = this;
        easternChunk->updateMesh();
    }
}

const int atlasX_size = 16;
const int atlasY_size = 1;

const float atlasTileX_Size = 1.0f / atlasX_size;
const float atlasTileY_Size = 1.0f / atlasY_size;

inline uint32_t compress_vec3int(Vector3Int vec)
{
    return (vec.x << 10) | (vec.y << 5) | (vec.z);
}

void VerticalChunkMesh::addSquare(Vector3Int pos, direction facing, uint16_t textureID)
{
    /*
          3____0
         /|   /|
        2_|__1 |
        | 7__|_4
        |/   |/
        6____5

        y  z
        |/__x
    */

    // Vector3 pos = pos_;

    // TODO measeure the atlas size and divide by atlas size
    float atlaspos = atlasTileX_Size * textureID;
    uint16_t VertexIndex = verticies.size();

    uint32_t new_verticies[6];

    switch (facing)
    {

    case direction::up: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(0, 1, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(1, 1, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(0, 1, 0));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(1, 1, 0));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;

    case direction::down: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(0, 0, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(1, 0, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(0, 0, 0));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(1, 0, 0));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;

    case direction::east: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(1, 0, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(1, 1, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(1, 0, 0));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(1, 1, 0));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;

    case direction::west: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(0, 0, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(0, 1, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(0, 0, 0));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(0, 1, 0));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;

    case direction::north: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(0, 1, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(1, 1, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(0, 0, 1));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(1, 0, 1));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;

    case direction::south: {
        new_verticies[0] = compress_vec3int(pos + Vector3Int(0, 1, 1));
        new_verticies[2] = compress_vec3int(pos + Vector3Int(1, 1, 1));
        new_verticies[1] = compress_vec3int(pos + Vector3Int(0, 0, 1));
        new_verticies[5] = compress_vec3int(pos + Vector3Int(1, 0, 1));

        new_verticies[3] = new_verticies[2];
        new_verticies[4] = new_verticies[1];
    }
    break;
    }

    verticies.insert(verticies.end(), new_verticies, new_verticies + 6);
}

void Chunk::TileRef::operator=(Tile tile)
{
    if (pos.y >= 0 && pos.y < max_block_height)
    {
        int slice = pos.y / chunk_size;
        if (auto& s = chunk.grid[slice])
            ((Tile*)s.get())[pos.x + ((pos.y % chunk_size) * chunk_size) + (pos.z * chunk_area)] = tile;
        else
        {
            s = std::make_unique<std::array<Tile, chunk_volume>>();
            for (auto& t : *s)
                t = air;
            ((Tile*)s.get())[pos.x + ((pos.y % chunk_size) * chunk_size) + (pos.z * chunk_area)] = tile;
        }
    }
    else
    {
        chunk.exc();
    }
}

Chunk::TileRef::operator Tile() const
{
    if (pos.y >= 0 && pos.y < max_block_height)
    {
        int slice = pos.y / chunk_size;
        if (auto& s = chunk.grid[slice])
            return ((Tile*)s.get())[pos.x + ((pos.y % chunk_size) * chunk_size) + (pos.z * chunk_area)];
    }
    return Tile(TileTypes::air);
}

// ChunksInRange
#define FUNC_ARGS *c, pos, pos + midPos

void ChunksInRange::East(Chunk* c, int range, Vector2Int pos)
{
    if (c)
    {
        if (range)
        {
            forEachChunk(FUNC_ARGS);
            East(c->easternChunk, range - 1, {pos.x + 1, pos.y});
        }
    }
    else
    {
        ifDoesntExist(FUNC_ARGS);
    }
}

void ChunksInRange::West(Chunk* c, int range, Vector2Int pos)
{
    if (c)
    {
        if (range)
        {
            forEachChunk(FUNC_ARGS);
            West(c->westernChunk, range - 1, {pos.x - 1, pos.y});
        }
    }
    else
    {
        ifDoesntExist(FUNC_ARGS);
    }
}

void ChunksInRange::South(Chunk* c, int range, Vector2Int pos)
{
    if (c)
    {
        if (range)
        {
            forEachChunk(FUNC_ARGS);
            South(c->southernChunk, range - 1, {pos.x, pos.y - 1});
            West(c->westernChunk, range - 1, {pos.x - 1, pos.y});
            East(c->easternChunk, range - 1, {pos.x + 1, pos.y});
        }
    }
    else
    {
        ifDoesntExist(FUNC_ARGS);
    }
}

void ChunksInRange::North(Chunk* c, int range, Vector2Int pos)
{
    if (c)
    {
        if (range)
        {
            forEachChunk(FUNC_ARGS);
            North(c->northernChunk, range - 1, {pos.x, pos.y + 1});
            West(c->westernChunk, range - 1, {pos.x - 1, pos.y});
            East(c->easternChunk, range - 1, {pos.x + 1, pos.y});
        }
    }
    else
    {
        ifDoesntExist(FUNC_ARGS);
    }
}

ChunksInRange::ChunksInRange(Chunk* c, func forEachChunk_, func ifDoesntExist_, int range)
{
    forEachChunk = forEachChunk_;
    ifDoesntExist = ifDoesntExist_;
    Vector2Int pos = {0, 0};
    if (c)
    {
        midPos = c->pos;
        forEachChunk(FUNC_ARGS);
        North(c->northernChunk, range - 1, {pos.x, pos.y + 1});
        South(c->southernChunk, range - 1, {pos.x, pos.y - 1});
        West(c->westernChunk, range - 1, {pos.x - 1, pos.y});
        East(c->easternChunk, range - 1, {pos.x + 1, pos.y});
    }
    else
    {
        ifDoesntExist(FUNC_ARGS);
    }
}
#undef FUNC_ARGS