#include "tile.hpp"

std::vector<TileProperties> Tile::TileTypes = {
    // clang-format off
    {true,  false, "air",   0},  //
    {false, true,  "stone", 1},  //
    {false, true,  "grass", 2},  //
    {false, true,  "dirt",  3},  //
    {false, true,  "sand",  4}   //
    //clang-format on
};

std::unordered_map<std::string, uint16_t> Tile::TileMap = {
    {"air", 0}, 
    {"stone", 1}, 
    {"grass", 2}, 
    {"dirt", 3},
    {"sand", 4}
    };

void Tile::construct(const std::string& name)
{
    ID = TileMap[name];
}
