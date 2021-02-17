#pragma once

#include <ostream>

class IndexBuffer
{
private:
    uint32_t bufferID;
public:
    uint32_t size;
private:
public:
    IndexBuffer() = default;
    IndexBuffer(const void* data, std::size_t size);
    ~IndexBuffer();

    inline IndexBuffer(IndexBuffer&& other) { bufferID = other.bufferID;other.bufferID = 0; }
    inline void operator =(IndexBuffer&& other) { bufferID = other.bufferID;other.bufferID = 0; }

    void Bind() const;
};