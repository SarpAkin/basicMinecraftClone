#include "game.h"

//CPP
void Game::initTerrainGen()
{
    tGen.setPos = [](Chunk& c, Vector2Int pos)
    {
        c.pos = pos;
    };


    float amp = 12.0f;
    float amp2 = 7.3f;
    double freq = .0137;
    double freq2 = .0692;
    auto heightNoise_ = std::make_shared<PerlinNoise>();
    auto heightNoise2_ = std::make_shared<PerlinNoise>();
    tGen.addRule(
        [=](Chunk& c)
        {
            auto& heightNoise = *heightNoise_;
            auto& heightNoise2 = *heightNoise2_;

            Vector3d RealCord = Vector3d(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);

            for (int z = 0;z < chunk_size;++z)
            {
                for (int x = 0;x < chunk_size;++x)
                {
                    int height = 23 + (int)(heightNoise[(RealCord + Vector3d(x, 0, z)) * freq] * amp + heightNoise2[(RealCord + Vector3d(x, 0, z))*freq2] * amp2);
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

        });

    //do Last
    tGen.Init();
}