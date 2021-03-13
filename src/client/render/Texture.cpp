#include "Texture.hpp"

#include "opengl_.hpp"

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
