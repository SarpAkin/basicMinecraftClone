#include "game.h"

//CPP
void Game::initTerrainGen()
{
    tGen.setPos = [](Chunk& c, Vector2Int pos)
    {
        c.pos = pos;
    };
    double amp = .3;
    tGen.addRule(
        [this, amp](Chunk& c)
    {
        Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
        for (int z = 0;z < chunk_size;++z)
        {
            for (int x = 0;x < chunk_size;++x)
            {
                int height = 16 + (int)(heightNoise[(RealCord + Vector3d(x, 0, z)) * amp] * 4.0f);
                for (int y = 0;y < height;++y)
                {
                    c.findBlockInChunk({ x,y,z }) = stone;
                }
            }
        }
    });

    tGen.addRule(
        [](Chunk& c)
        {
            for (int x = 0;x < chunk_size;++x)
                for (int z = 0;z < chunk_size;++z)
                {
                    int lastAir = max_block_height;
                    for (int y = max_block_height - 1;y >= 0;--y)
                    {
                        if(Tile tile = c.findBlockInChunk({x,y,z}))
                        {
                            int dif = lastAir - y;
                            if(tile == stone && dif < 5)
                            {
                                c.findBlockInChunk({x,y,z}) = dif == 1 ? grass : dirt;
                            }
                        }
                        else
                        {
                            lastAir = y;
                        }
                    }
                }

        });

    //do Last
    tGen.Init();
}