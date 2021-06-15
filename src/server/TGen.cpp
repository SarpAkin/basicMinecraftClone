#include "TGen.hpp"

#include <algorithm>
#include <chrono>
#include <random>

#include "../common/chunk.hpp"
#include "../common/noise.hpp"

void CreateNewChunkStage(const Chunk& cOld, Chunk& cNew)
{
    cNew.pos = cOld.pos;
    // std::cout << cOld.pos.x << ' ' << cOld.pos.y << '\n';
    cNew.chunkGenData = std::make_unique<ChunkGenData>(*cOld.chunkGenData);
}

void TGen::GenerateChunk(Vector2Int pos)
{
    GenListMutex.lock();
    if (generatingChunks.find(pos) == generatingChunks.end())
    {
        generatingChunks[pos] = 1;
        TGenInMut.lock();
        chunksToGenerateList.push_back(pos);
        TGenInMut.unlock();
        TGenWait.notify_one();
    }
    GenListMutex.unlock();

    // std::cout << pos.x << ' ' << pos.y << '\n';
}

void TGen::GenerateChunks()
{
    while (running)
    {
        while (true)
        {
            // std::cin.get();
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

std::vector<std::unique_ptr<Chunk>> TGen::GetChunks()
{
    TGenOutMut.lock();
    auto tmp = std::move(TGenOutChunks);
    GenListMutex.lock();
    for (auto& c : tmp)
    {
        generatingChunks.erase(c->pos);
    }
    GenListMutex.unlock();
    TGenOutMut.unlock();
    return tmp;
}

ModifyCurrent<Chunk> TGen::iterateAllBlocks(std::function<void(Tile&, Vector3Int pos)> func)
{
    auto retfunc = [=](Chunk& c) {
        auto RealCord = Vector3Int(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
        auto BlockCord = RealCord;
        for (int w = 0; w < vertical_chunk_count; ++w)
            if (c.grid[w])
            {
                auto& grid = *c.grid[w];
                int index = 0;
                for (int z = 0; z < chunk_size; ++z)
                {
                    BlockCord.z = z + RealCord.z;
                    for (int y = w * chunk_size, y_end = y + chunk_size; y < y_end; ++y)
                    {
                        BlockCord.y = y + RealCord.y;
                        BlockCord.x = RealCord.x;
                        for (int x = 0; x < chunk_size; ++x, ++index)
                        {
                            func(grid[index], BlockCord);
                            ++BlockCord.x;
                        }
                    }
                }
            }
    };
    return retfunc;
}

void TGen::initTgen(TerrainGenerator<Chunk>& tGen)
{
    tGen.setPos = [](Chunk& c, Vector2Int pos) { c.pos = pos; };

    auto place_blocks_to_place = [](nearbyChunks<Chunk> n) -> std::unique_ptr<Chunk> {
        auto& old_chunk = *n[1][1];
        Vector2Int current_cpos = old_chunk.pos;

        auto new_chunk = std::make_unique<Chunk>(current_cpos);
        CreateNewChunkStage(old_chunk, *new_chunk);

        for (int i = 0; i < vertical_chunk_count; ++i)
        {
            if (old_chunk.grid[i])
                new_chunk->grid[i] = std::make_unique<std::array<Tile, chunk_volume>>(*old_chunk.grid[i]);
        }
        for (auto& n_ : n)
            for (auto& c : n_)
            {
                Vector2Int boundry = (current_cpos - c->pos) * chunk_size;

                Vector3Int offset = {-boundry.x, 0,-boundry.y};
                for (auto b : c->chunkGenData->blocks_to_place)
                {
                    Vector3Int bpos = std::get<0>(b);
                    // clang-format off
                    if (bpos.x < boundry.x + chunk_size && bpos.x >= boundry.x 
                     && bpos.z < boundry.y + chunk_size && bpos.z >= boundry.y)
                    // clang-format on
                    {
                        (*new_chunk).findBlockInChunk(Vector3Int(bpos.x, bpos.y, bpos.z) + offset) = std::get<1>(b);
                    }
                }
            }
        return new_chunk;
    };

    auto permutation_table = std::make_shared<std::array<uint8_t, 256 * 256>>();
    for (auto& num : *permutation_table)
        num = rand();

    auto BiomeNoise = std::make_shared<PerlinNoise>();
    tGen.addRule([=](Chunk& c) {
        Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
        c.chunkGenData = std::make_unique<ChunkGenData>();
        auto& bMap = c.chunkGenData->biomeMap;
        for (int z = 0; z < chunk_size; ++z)
            for (int x = 0; x < chunk_size; ++x)
            {
                bMap[x + z * chunk_size] = (*BiomeNoise)[(RealCord + Vector3d(x, 0, z)) * 0.0179] > -.2;
            }
    });
    /*
    tGen.addRule(
        [](Chunk& c)
        {
            Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
            for (int z = 0;z < chunk_size;++z)
                for (int x = 0;x < chunk_size;++x)
                    c[{x,0,z}] = x == 0 || z == 0 ? stone : (c.chunkGenData->biomeMap[z*chunk_size+x] == 0 ? grass :
    sand);
        }
    );
    */

    float amp_ = 9.0f;
    float amp2 = 17.3f;
    double freq = .00237;
    double freq2 = .0692;
    auto heightNoise_ = std::make_shared<PerlinNoise>();
    auto heightNoise2_ = std::make_shared<PerlinNoise>();
    tGen.addRule([=](Chunk& c) {
        // float amp_ = amp;
        auto& heightNoise = *heightNoise_;
        auto& heightNoise2 = *heightNoise2_;

        Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);

        for (int z = 0; z < chunk_size; ++z)
        {
            for (int x = 0; x < chunk_size; ++x)
            {
                float amp;
                if (c.chunkGenData->biomeMap[x + z * chunk_size] == 1)
                    amp = amp_ * 2.9f;
                else
                    amp = amp_;

                auto BlockCord = RealCord + Vector3d(x, 0, z);
                int BaseHeight =
                    33 + (int)(heightNoise[BlockCord * freq] * amp + heightNoise2[BlockCord * freq2] * amp2);
                int MaxHeight = std::min(BaseHeight + 15, max_block_height);
                int MinHeight = std::max(BaseHeight - 15, 0);
                for (int y = MinHeight; y < MaxHeight; ++y)
                {
                    BlockCord.y = y;

                    // this is to create cliffs
                    // clang-format off
                    c.findBlockInChunk({x, y, z}) =
                        (23 + (int)(heightNoise[
                            (RealCord + Vector3d(
                                x + heightNoise2[BlockCord * freq2] * amp2, 
                                0,
                                z + heightNoise2[Vector3d(BlockCord.z, BlockCord.y, BlockCord.x) * freq2]
                                * amp2)) * freq] * amp)) > y ? stone : air;
                    // clang-format on
                }
                // Fill till MinHeigt
                for (int y = 0; y < MinHeight; ++y)
                    c.findBlockInChunk({x, y, z}) = stone;
            }
        }
    });

    // TODO do some optimaztions for this
    /*auto smoothFunc = [](nearbyChunks<Chunk> n) -> std::unique_ptr<Chunk>
    {
        auto newChunk = std::make_unique<Chunk>();
        auto& c = *newChunk;
        auto& midChunk_ = *n[1][1];
        CreateNewChunkStage(midChunk_, c);
        const auto& midChunk = midChunk_;
        Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
        for (int w = 0;w < vertical_chunk_count;++w)
            if (midChunk.grid[w])
                for (int x = 0;x < chunk_size;++x)
                    for (int z = 0;z < chunk_size;++z)
                        for (int y = w * chunk_size, y_end = y + chunk_size;y < y_end;++y)
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
    */
    auto smoothFuncOptimezed = [](nearbyChunks<Chunk> n) -> std::unique_ptr<Chunk> {
        auto newChunk = std::make_unique<Chunk>();
        auto& c = *newChunk;
        const auto& midChunk = *n[1][1];
        CreateNewChunkStage(midChunk, c);

        const int range = 2;
        const int threshold = 70;

        const int rangeStart = -range;
        const int rangeEnd = range + 1;
        const int chunkRangeEnd = chunk_size - range;

        for (int w = 0; w < vertical_chunk_count; ++w)
            if (midChunk.grid[w])
            {
                auto oldGrid = (Tile*)midChunk.grid[w].get();
                c.grid[w] = std::make_unique<std::array<Tile, chunk_volume>>();
                auto newGrid = (Tile*)c.grid[w].get();
                for (int z = range; z < chunkRangeEnd; ++z)
                {
                    int m_z = z * chunk_area;
                    for (int y = range, y_end = chunkRangeEnd; y < y_end; ++y)
                    {
                        int index = range + m_z + y * chunk_size;
                        for (int x = range; x < chunkRangeEnd; ++x, ++index)
                        {
                            int counter = 0;
                            for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                            {
                                int m_z_ = (z + z_) * chunk_area;
                                for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                {
                                    int index_ = x + rangeStart + m_z_ + (y + y_) * chunk_size;
                                    for (int x_ = rangeStart; x_ < rangeEnd; ++x_, ++index_)
                                    {
                                        counter += oldGrid[index_] == stone;
                                    }
                                }
                            }
                            newGrid[index] = counter > threshold ? stone : air;
                        }
                        // set y to real cord
                        y += w * chunk_size;
                        for (int x = 0; x < range; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }

                        for (int x = chunkRangeEnd; x < chunk_size; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }
                        // set y back to old cord since its shared
                        y -= w * chunk_size;
                    }

                    for (int y = w * chunk_size, y_end = y + range; y < y_end; ++y)
                        for (int x = 0; x < chunk_size; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }

                    for (int y = w * chunk_size + chunkRangeEnd, y_end = y + range; y < y_end; ++y)
                        for (int x = 0; x < chunk_size; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }
                }
                for (int z = 0; z < range; ++z)
                    for (int y = w * chunk_size, y_end = y + chunk_size; y < y_end; ++y)
                        for (int x = 0; x < chunk_size; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }

                for (int z = chunkRangeEnd; z < chunk_size; ++z)
                    for (int y = w * chunk_size, y_end = y + chunk_size; y < y_end; ++y)
                        for (int x = 0; x < chunk_size; ++x)
                        {
                            int counter = 0;
                            for (int x_ = rangeStart; x_ < rangeEnd; ++x_)
                                for (int z_ = rangeStart; z_ < rangeEnd; ++z_)
                                    for (int y_ = rangeStart; y_ < rangeEnd; ++y_)
                                        counter += midChunk[{x + x_, y + y_, z + z_}] == stone;
                            c[{x, y, z}] = counter > threshold ? stone : air;
                        }
            }
        return newChunk;
    };
    // for(int i = 0;i < 2;++i)
    tGen.addRule(smoothFuncOptimezed);
    // tGen.addRule(smoothFunc);

    tGen.addRule([](Chunk& c) {
        for (int x = 0; x < chunk_size; ++x)
            for (int z = 0; z < chunk_size; ++z)
            {
                if (c.chunkGenData->biomeMap[x + z * chunk_size])
                {
                    int lastAir = max_block_height;
                    for (int y = max_block_height - 1; y >= 0; --y)
                    {
                        if (Tile tile = c.findBlockInChunk({x, y, z}))
                        {
                            int dif = lastAir - y;
                            if (tile == stone && dif < 5)
                            {
                                c.findBlockInChunk({x, y, z}) = dif == 1 ? grass : dirt;
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
                    for (int y = max_block_height - 1; y >= 0; --y)
                    {
                        if (Tile tile = c.findBlockInChunk({x, y, z}))
                        {
                            int dif = lastAir - y;
                            if (tile == stone && dif < 5)
                            {
                                c.findBlockInChunk({x, y, z}) = sand;
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

    auto CaveNoise = std::make_shared<PerlinNoise>();
    tGen.addRule(TGen::iterateAllBlocks([=](Tile& t, Vector3Int pos) {
        if ((*CaveNoise)[((Vector3d)pos) * 0.0617] > .4f)
            t = air;
    }));

    uint16_t wood = Tile::TileMap["wood"];
    uint16_t leaf = Tile::TileMap["leaf"];

    auto tree_stump_rule = [wood, leaf, permutation_table](Chunk& c) {
        for (int x = 0; x < chunk_size; ++x)
        {
            for (int z = 0; z < chunk_size; ++z)
            {
                int xz_offset = z * chunk_area + x;
                for (int y_vert_c = vertical_chunk_count - 1; y_vert_c >= 0; --y_vert_c)
                {
                    if (c.grid[y_vert_c] == nullptr)
                        continue;

                    auto& grid = *c.grid[y_vert_c];
                    for (int y = chunk_size - 1; y >= 0; --y)
                    {
                        if (grid[xz_offset + y * chunk_size] != air)
                        {
                            if (grid[xz_offset + y * chunk_size] == grass &&
                                (*permutation_table)[(x + z * 64 + c.pos.x * 57 + c.pos.y * 103) & 0xFFFF] > 253)
                            {
                                // c.chunkGenData->tree_stump_poses.emplace_back(x, y, z);
                                auto& tree_blocks = c.chunkGenData->blocks_to_place;

                                int tree_height = 8;

                                for (int i = 1; i < tree_height; ++i)
                                {
                                    tree_blocks.emplace_back(std::make_tuple(            //
                                        Vector3Int(x, y + y_vert_c * chunk_size + i, z), //
                                        Tile(wood)));                                    //
                                }

                                for (int x_ = -2; x_ <= 2; ++x_)
                                    for (int y_ = 4; y_ <= tree_height + 2; ++y_)
                                        for (int z_ = -2; z_ <= 2; ++z_)
                                        {
                                            if (x_ != 0 || z_ != 0 || y_ >= tree_height)
                                            {
                                                tree_blocks.emplace_back(std::make_tuple(
                                                    Vector3Int(x + x_, y + y_vert_c * chunk_size + y_, z + z_),
                                                    Tile(leaf)));
                                            }
                                        }

                                
                            }

                            // break out of nested loop
                            goto y_loop_exit;
                        }
                    }
                }
            y_loop_exit:;
            }
        }
    };

    tGen.addRule(tree_stump_rule);
    tGen.addRule(place_blocks_to_place);

    // tGen.addRule([wood, permutation_table](Chunk& c) {
    //     for (auto& tree : c.chunkGenData->tree_stump_poses)
    //     {
    //         int tree_height = 4 + (*permutation_table)[tree.x + tree.y + tree.z] % 3;
    //         if (tree_height + tree.y >= chunk_size * vertical_chunk_count)
    //             continue;

    //         for (int y = tree.y + 1; y <= tree.y + tree_height; ++y)
    //         {
    //             c[{tree.x, y, tree.z}] = wood;
    //         }
    //     }
    // });
    /*
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
    );*/

    tGen.addRule([](Chunk& c) {
        c.resetNeighbour();
        c.chunkGenData = nullptr;
    });
    // do Last
    tGen.Init();
}
