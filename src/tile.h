#pragma once

#include <inttypes.h>
#include <vector>
#include <string>
#include <unordered_map>

struct TileProperties
{
    bool isTransparent;
    std::string name;
    uint16_t TextureID;

};

enum TileTypes : uint16_t
{
    air = 0,
    stone = 1,
    grass = 2,
    dirt = 3,
    sand = 4
};

struct Tile
{
private:
    /*helper function for constructing Tile from a tilename*/
    void construct(const std::string& Tilename);
public:

    uint16_t ID = 0;
    Tile() = default;
    inline Tile(uint16_t ID_) { ID = ID_; }
    Tile(const std::string& Tilename){construct(Tilename);};
    inline Tile(const char* Tilename){std::string name = Tilename;construct(name);}

    inline operator uint16_t& () { return ID; }
    inline operator const uint16_t& () const { return ID; }
    inline const TileProperties& properties () const {return TileTypes[ID];}

    static std::vector<TileProperties> TileTypes;
    static std::unordered_map<std::string, uint16_t> TileMap;

};
