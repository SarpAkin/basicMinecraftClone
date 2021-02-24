#include "tile.h"

std::vector<TileProperties> Tile::TileTypes = 
{
    {true,"air",0},
    {false,"stone",0},
    {false,"grass",1},
    {false,"dirt",2}
};

std::unordered_map<std::string, uint16_t> Tile::TileMap = 
{
    {"air",0},
    {"stone",1},
    {"grass",2},
    {"dirt",3}
};

void Tile::construct(const std::string& name)
{
    ID = TileMap[name];
}