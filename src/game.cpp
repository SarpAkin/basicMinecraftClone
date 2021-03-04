#include "game.h"

//CPP

Game::~Game()
{
    std::cout << "stopping!\n";
    running = false;
    TGenWait.notify_all();
    for (auto& t : TGenThreads)
    {
        t.join();
    }

}

void Game::Tick()
{
    TGenOutMut.lock();
    for (auto& c : TGenOutChunks)
    {
        auto pos = c->pos;
        //c->resetNeighbour();
        c->Init(chunks);
        chunks[pos] = std::move(c);
        generatingChunks.erase(pos);
        //std::cout << pos.x << ' ' << pos.y << '\n';
    }
    TGenOutChunks.clear();
    TGenOutMut.unlock();
}

void Game::genChunks()
{
    while (running)
    {
        while (true)
        {
            //std::cin.get();
            TGenInMut.lock();
            if (chunksToGenerateList.size())
            {
                auto pos = chunksToGenerateList.back();
                chunksToGenerateList.pop_back();
                TGenInMut.unlock();

                auto tmp = tGen.GenerateChunk(pos);

                TGenOutMut.lock();
                TGenOutChunks.push_back(std::move(tmp));
                TGenOutMut.unlock();
            }
            else
            {
                TGenInMut.unlock();
                break;
            }
        }
        TGenWait.wait(TGenWaitMutex);
    }

}

void Game::GenerateChunk(Vector2Int pos)
{
    if (!chunks[pos])
    {
        if (generatingChunks.find(pos) == generatingChunks.end())
        {
            generatingChunks[pos] = 1;
            TGenInMut.lock();
            chunksToGenerateList.push_back(pos);
            TGenInMut.unlock();
            TGenWait.notify_one();
        }
    }
}

void CreateNewChunkStage(const Chunk& cOld, Chunk& cNew)
{
    cNew.pos = cOld.pos;
    //std::cout << cOld.pos.x << ' ' << cOld.pos.y << '\n';
    cNew.chunkGenData = std::make_unique<ChunkGenData>(*cOld.chunkGenData);
}

void Game::initTerrainGen()
{
    tGen.setPos = [](Chunk& c, Vector2Int pos)
    {
        c.pos = pos;
    };


    auto BiomeNoise = std::make_shared<PerlinNoise>();
    tGen.addRule(
        [=](Chunk& c)
        {
            Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
            c.chunkGenData = std::make_unique<ChunkGenData>();
            auto& bMap = c.chunkGenData->biomeMap;
            for (int z = 0;z < chunk_size;++z)
                for (int x = 0;x < chunk_size;++x)
                {
                    bMap[x + z * chunk_size] = (*BiomeNoise)[(RealCord + Vector3d(x, 0, z)) * 0.0179] > -.2;
                }
        }
    );
    /*
    tGen.addRule(
        [](Chunk& c)
        {
            Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
            for (int z = 0;z < chunk_size;++z)
                for (int x = 0;x < chunk_size;++x)
                    c[{x,0,z}] = x == 0 || z == 0 ? stone : (c.chunkGenData->biomeMap[z*chunk_size+x] == 0 ? grass : sand);
        }
    );
    */

    float amp_ = 9.0f;
    float amp2 = 17.3f;
    double freq = .0237;
    double freq2 = .0692;
    auto heightNoise_ = std::make_shared<PerlinNoise>();
    auto heightNoise2_ = std::make_shared<PerlinNoise>();
    tGen.addRule(
        [=](Chunk& c)
        {
            //float amp_ = amp;
            auto& heightNoise = *heightNoise_;
            auto& heightNoise2 = *heightNoise2_;

            Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);

            for (int z = 0;z < chunk_size;++z)
            {
                for (int x = 0;x < chunk_size;++x)
                {
                    float amp;
                    if (c.chunkGenData->biomeMap[x + z * chunk_size] == 1)
                        amp = amp_ * 2.9f;
                    else
                        amp = amp_;

                    auto BlockCord = RealCord + Vector3d(x, 0, z);
                    int BaseHeight = 23 + (int)(heightNoise[BlockCord * freq] * amp + heightNoise2[BlockCord * freq2] * amp2);
                    int MaxHeight = std::min(BaseHeight + 15, max_block_height);
                    int MinHeight = std::max(BaseHeight - 15, 0);
                    for (int y = MinHeight;y < MaxHeight;++y)
                    {
                        BlockCord.y = y;
                        //this is to create cliffs
                        c.findBlockInChunk({ x,y,z }) = (23 + (int)(heightNoise[(RealCord + Vector3d(
                            x + heightNoise2[BlockCord * freq2] * amp2,
                            0,
                            z + heightNoise2[Vector3d(BlockCord.z, BlockCord.y, BlockCord.x) * freq2] * amp2)
                            ) * freq] * amp)) > y ? stone : air;
                    }
                    //Fill till MinHeigt
                    for (int y = 0;y < MinHeight;++y)
                        c.findBlockInChunk({ x,y,z }) = stone;
                }
            }
        });


    //TODO do some optimaztions for this
    auto smoothFunc = [](nearbyChunks<Chunk> n) -> std::unique_ptr<Chunk>
    {
        auto newChunk = std::make_unique<Chunk>();
        auto& c = *newChunk;
        auto& midChunk_ = *n[1][1];
        CreateNewChunkStage(midChunk_, c);
        const auto& midChunk = midChunk_;
        Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
        for (int x = 0;x < chunk_size;++x)
            for (int z = 0;z < chunk_size;++z)
                for (int y = 0;y < max_block_height;++y)
                {
                    int counter = 0;
                    const int rangeStart = -2;
                    const int rangeEnd = 3;
                    for (int x_ = rangeStart;x_ < rangeEnd;++x_)
                        for (int z_ = rangeStart;z_ < rangeEnd;++z_)
                            for (int y_ = rangeStart;y_ < rangeEnd;++y_)
                                counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                    c[{x, y, z}] = counter > 70 ? stone : air;
                }
        return newChunk;
    };
    //for(int i = 0;i < 4;++i)
    tGen.addRule(smoothFunc);
    //tGen.addRule(smoothFunc);

    tGen.addRule(
        [](Chunk& c)
        {
            for (int x = 0;x < chunk_size;++x)
                for (int z = 0;z < chunk_size;++z)
                {
                    if (c.chunkGenData->biomeMap[x + z * chunk_size])
                    {
                        int lastAir = max_block_height;
                        for (int y = max_block_height - 1;y >= 0;--y)
                        {
                            if (Tile tile = c.findBlockInChunk({ x,y,z }))
                            {
                                int dif = lastAir - y;
                                if (tile == stone && dif < 5)
                                {
                                    c.findBlockInChunk({ x,y,z }) = dif == 1 ? grass : dirt;
                                }
                            }
                            else
                            {
                                lastAir = y;
                            }
                        }
                    }
                    else
                    {
                        int lastAir = max_block_height;
                        for (int y = max_block_height - 1;y >= 0;--y)
                        {
                            if (Tile tile = c.findBlockInChunk({ x,y,z }))
                            {
                                int dif = lastAir - y;
                                if (tile == stone && dif < 5)
                                {
                                    c.findBlockInChunk({ x,y,z }) = sand;
                                }
                            }
                            else
                            {
                                lastAir = y;
                            }
                        }
                    }
                }
        });


    /*
        auto CaveNoise = std::make_shared<PerlinNoise>();
        tGen.addRule(
            [=](Chunk& c)
            {
                Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
                for (int x = 0;x < chunk_size;++x)
                    for (int z = 0;z < chunk_size;++z)
                        for (int y = 0;y < max_block_height;++y)
                            if ((*CaveNoise)[(RealCord + Vector3d(x, y, z)) * 0.0617] > .4f)
                                c[{x, y, z}] = air;

            }
        );
    */

    tGen.addRule(
        [](Chunk& c)
        {
            c.resetNeighbour();
        }
    );
    //do Last
    tGen.Init();
}