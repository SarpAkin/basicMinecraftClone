#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Entity.hpp"
#include "chunk.hpp"
#include "hasher.hpp"
#include "noise.hpp"
#include "vectors.hpp"

class Game
{
private:

    // Entity spawning and storing
    // EntityID entityIDCounter = 0;
    std::vector<std::weak_ptr<Entity>> Entities;
    std::vector<EntityID> deletedEntityPositions;
    //
    bool running;

public:
    std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>> chunks;

private:
    void initTerrainGen();
    void genChunks();
    
    virtual std::shared_ptr<Entity> GetEntity();
public: 
    void GenerateChunk(Vector2Int pos);
    std::shared_ptr<Entity> SpawnEntity(std::unique_ptr<Entity>);
    std::shared_ptr<Entity> SpawnEntity(std::unique_ptr<Entity> e, Chunk& c);

    ~Game();
    Game();
};
