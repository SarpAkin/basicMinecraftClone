#pragma once

#include <unordered_map>
#include <memory>

#include "TerrainGen.h"
#include "vectors.h"
#include "hasher.h"
#include "noise.h"
#include "chunk.h"

class Game
{
private:
    TerrainGenerator<Chunk> tGen;
    PerlinNoise heightNoise;
public:
    std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>> chunks;

private:
    void initTerrainGen();
public:
    inline void GenerateChunk(Vector2Int pos)
    {
        auto tmp = tGen.GenerateChunk(pos);
        tmp->Init(chunks);
        chunks[pos] = std::move(tmp);
    }
    
    inline Game()
    {
        initTerrainGen();
    }
};