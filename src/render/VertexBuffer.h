#pragma once

#include <ostream>
#include <vector>
#include <memory>

class VertexArray;

class VertexBuffer
{
    friend VertexArray;
private:
    uint32_t bufferID = 0;

private:
public:
    VertexBuffer() = default;
    void Construct(const void* data, std::size_t size);
    inline VertexBuffer(const void* data, std::size_t size)
    {
        Construct(data, size);
    }
    ~VertexBuffer();

    inline VertexBuffer(VertexBuffer&& other) { bufferID = other.bufferID;other.bufferID = 0; }
    inline void operator =(VertexBuffer&& other) { this->~VertexBuffer();bufferID = other.bufferID;other.bufferID = 0; }


    template<typename T>
    inline VertexBuffer(std::vector<T>& vec)
    {
        if (vec.size())
            Construct(&(vec[0]), vec.size() * sizeof(T));
    }

    void Bind() const;
};