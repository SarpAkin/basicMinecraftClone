#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "TerrainGen.hpp"

#include "../common/Entity.hpp"
#include "../common/chunk.hpp"
#include "../common/hasher.hpp"

class TGen
{
private:
    TerrainGenerator<Chunk> tGen;
    static const int TGenWorkerThreadCount = 1;
    std::vector<std::thread> TGenThreads;
    std::vector<Vector2Int> chunksToGenerateList;
    std::vector<std::unique_ptr<Chunk>> TGenOutChunks;
    std::unordered_map<Vector2Int, bool, Hasher<Vector2Int>, Equal<Vector2Int>> generatingChunks;
    std::mutex TGenInMut;
    std::mutex TGenOutMut;
    std::mutex TGenWaitMutex;
    std::mutex GenListMutex;
    std::condition_variable_any TGenWait;

    bool running = true;

private:
    void GenerateChunks();

public:
    std::vector<std::unique_ptr<Chunk>> GetChunks();
    void GenerateChunk(Vector2Int pos);
    inline void GenerateChunk(std::vector<Vector2Int> poses){for(auto& pos : poses)GenerateChunk(pos);}

    static ModifyCurrent<Chunk> iterateAllBlocks(std::function<void(Tile&, Vector3Int pos)> func);
    static void initTgen(TerrainGenerator<Chunk>& tGen);

    inline TGen()
    {
        initTgen(tGen);
        running = true;
        for (int i = 0; i < TGenWorkerThreadCount; ++i)
            TGenThreads.emplace_back(&TGen::GenerateChunks, this);
    }

    inline ~TGen()
    {
        running = false;
        TGenWait.notify_all();
        for (auto& t : TGenThreads)
        {
            t.join();
        }
    }
};
