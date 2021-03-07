#include "chunk.h"
#include "ChunkMeshGPU.h"

#include <iostream>

Chunk::TileRef Chunk::operator[] (Vector3Int pos)
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

Tile Chunk::operator[] (Vector3Int pos) const
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

    //return air if it can't find
    return Tile(TileTypes::air);
}

void Chunk::updateMesh()
{
    delete GPUMesh;
    GPUMesh = nullptr;
}

ChunkMesh Chunk::GenMesh() const
{
    ChunkMesh mesh_;
    auto& this_ = (*this);
    for (int i = 0;i < vertical_chunk_count;++i)
    {
        VerticalChunkMesh& mesh = mesh_.meshes[i];
        if (grid[i] != nullptr)
        {
            mesh.reserverSquares(1000);
            for (int y = 0;y < chunk_size;++y)
            {
                int globalY = y + i * chunk_size;
                for (int x = 0;x < chunk_size;++x)
                {
                    for (int z = 0;z < chunk_size;++z)
                    {
                        auto tile = this_[{x, globalY, z}];
                        auto textureID = tile.properties().TextureID;
                        if (tile.ID)
                        {
                            if (this_[{x, globalY + 1, z}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::up, textureID);

                            if (this_[{x, globalY - 1, z}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::down, textureID);

                            if (this_[{x, globalY, z + 1}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::north, textureID);

                            if (this_[{x, globalY, z - 1}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::south, textureID);

                            if (this_[{x + 1, globalY, z}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::east, textureID);

                            if (this_[{x - 1, globalY, z}].properties().isTransparent)
                                mesh.addSquare({ x,y,z }, direction::west, textureID);
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
    delete GPUMesh;
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

const float  atlasTileX_Size = 1.0f / atlasX_size;
const float atlasTileY_Size = 1.0f / atlasY_size;

void VerticalChunkMesh::addSquare(Vector3Int pos_, direction facing, uint16_t textureID)
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

    Vector3 pos = pos_;

    //TODO measeure the atlas size and divide by atlas size
    float atlaspos = atlasTileX_Size * textureID;
    uint16_t VertexIndex = verticies.size();
    switch (facing)
    {


    case direction::up:
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, -.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, 0.5f), Vector2(atlaspos, 1));//4
        break;

    case direction::down:
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, 0.5f), Vector2(atlaspos, 1));//4
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, -.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        break;

    case direction::east:
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, 0.5f), Vector2(atlaspos, 1));//4
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, -.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        break;

    case direction::west:
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, -.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, 0.5f), Vector2(atlaspos, 1));//4
        break;

    case direction::north:
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, 0.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, 0.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, 0.5f), Vector2(atlaspos, 1));//4
        break;

    case direction::south:
        verticies.emplace_back(pos + Vector3(-.5f, 0.5f, -.5f), Vector2(atlaspos, 1));//4
        verticies.emplace_back(pos + Vector3(-.5f, -.5f, -.5f), Vector2(atlaspos, 0));//3
        verticies.emplace_back(pos + Vector3(0.5f, -.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 0));//2
        verticies.emplace_back(pos + Vector3(0.5f, 0.5f, -.5f), Vector2(atlaspos + atlasTileX_Size, 1));//1
        break;
    }
    indicies.push_back(VertexIndex + 0);
    indicies.push_back(VertexIndex + 1);
    indicies.push_back(VertexIndex + 3);

    indicies.push_back(VertexIndex + 1);
    indicies.push_back(VertexIndex + 2);
    indicies.push_back(VertexIndex + 3);
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