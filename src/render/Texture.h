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

    inline Texture(Texture&& other) { TextureID = other.TextureID;other.TextureID = 0; }
    inline void operator =(Texture&& other) { TextureID = other.TextureID;other.TextureID = 0; }

    inline int Width()  const { return width; }
    inline int Height() const { return height; }
};


//CPP
#include "opengl_.h"

void Texture::construct(const std::string& path)
{
    stbi_set_flip_vertically_on_load(1);

    u_char* localBuffer;
    localBuffer = stbi_load(path.c_str(), &width, &height, &BPP, 4);


    GLCALL(glGenTextures(1, &TextureID));
    GLCALL(glBindTexture(GL_TEXTURE_2D, TextureID));

    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer));
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

    if (localBuffer)
        stbi_image_free(localBuffer);
}

Texture::~Texture()
{
    if (TextureID)
        GLCALL(glDeleteTextures(1, &TextureID));
}

void Texture::Bind(uint32_t slot) const
{
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, TextureID));
}