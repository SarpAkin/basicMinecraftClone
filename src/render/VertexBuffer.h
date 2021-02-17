#pragma once


#include <ostream>
#include <memory>

class VertexArray;

class VertexBuffer
{
    friend VertexArray;
private:
    uint32_t bufferID;

private:
public:
    VertexBuffer() = default;
    VertexBuffer(const void* data, std::size_t size);
    ~VertexBuffer();

    inline VertexBuffer(VertexBuffer&& other) { bufferID = other.bufferID;other.bufferID = 0; }
    inline void operator =(VertexBuffer&& other) { bufferID = other.bufferID;other.bufferID = 0; }

    void Bind() const;
};