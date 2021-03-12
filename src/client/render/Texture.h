#pragma once

#include <stb_image.h>
#include <stdint.h>
#include <string>

class Texture
{
private:
    uint32_t TextureID = 0;
    std::string filePath;
    int width, height, BPP;
    void construct(const std::string& path);
public:
    Texture() = default;
    inline Texture(const std::string& path) { construct(path); }
    inline Texture(const char* path) { std::string path_ = path;construct(path_); }
    ~Texture();

    void Bind(uint32_t slot = 0) const;

    inline Texture(Texture&& other)
    {
        this->~Texture();
        width = other.width;
        height = other.height;
        BPP = other.BPP;
        TextureID = other.TextureID;
        other.TextureID = 0;
    }
    inline void operator =(Texture&& other)
    {
        this->~Texture();
        width = other.width;
        height = other.height;
        BPP = other.BPP;
        TextureID = other.TextureID;
        other.TextureID = 0;
    }

    inline int Width()  const { return width; }
    inline int Height() const { return height; }
};


