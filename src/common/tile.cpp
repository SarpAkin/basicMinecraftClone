#include "tile.hpp"

std::vector<TileProperties> Tile::TileTypes = 
{
    {true,"air",0},
    {false,"stone",0},
    {false,"grass",1},
    {false,"dirt",2},
    {false,"sand",3}
};

std::unordered_map<std::string, uint16_t> Tile::TileMap = 
{
    {"air",0},
    {"stone",1},
    {"grass",2},
    {"dirt",3},
    {"sand",4}
};

void Tile::construct(const std::string& name)
{
    ID = TileMap[name];
}
