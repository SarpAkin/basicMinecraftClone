#pragma once

#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>

#include "TerrainGen.h"
#include "vectors.h"
#include "hasher.h"
#include "noise.h"
#include "chunk.h"

class Game
{
private:
    //TGen related
    TerrainGenerator<Chunk> tGen;
    static const int TGenWorkerThreadCount = 1;
    std::vector<std::thread> TGenThreads;
    std::vector<Vector2Int> chunksToGenerateList;
    std::vector<std::unique_ptr<Chunk>> TGenOutChunks;
    std::unordered_map<Vector2Int, bool, Hasher<Vector2Int>, Equal<Vector2Int>> generatingChunks;
    std::mutex TGenInMut;
    std::mutex TGenOutMut;
    std::mutex TGenWaitMutex;
    std::condition_variable_any TGenWait;
    //
    bool running;
public:
    std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>> chunks;

private:
    void initTerrainGen();
    void genChunks();
public:
    void Tick();
    void GenerateChunk(Vector2Int pos);
    ~Game();

    inline Game()
    {
        running = true;
        
        for (int i = 0;i < TGenWorkerThreadCount;++i)
            TGenThreads.emplace_back(&Game::genChunks, this);
        initTerrainGen();
    }


};
