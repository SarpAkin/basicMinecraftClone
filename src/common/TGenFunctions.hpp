#pragma once

#include <functional>

#include "TerrainGen.hpp"
#include "chunk.hpp"


class TGenFunctions
{
private:
    /* data */
public:
    static ModifyCurrent<Chunk> iterateAllBlocks(std::function<void(Tile&, Vector3Int pos)> func)
    {
        auto retfunc = [=](Chunk& c)
        {
            auto RealCord = Vector3Int(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
            auto BlockCord = RealCord;
            for (int w = 0;w < vertical_chunk_count;++w)
                if (c.grid[w])
                {
                    auto& grid = *c.grid[w];
                    int index = 0;
                    for (int z = 0;z < chunk_size;++z)
                    {
                        BlockCord.z = z + RealCord.z;
                        for (int y = w * chunk_size, y_end = y + chunk_size;y < y_end;++y)
                        {
                            BlockCord.y = y + RealCord.y;
                            BlockCord.x = RealCord.x;
                            for (int x = 0;x < chunk_size;++x, ++index)
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
};
