#pragma once

#include <ostream>
#include <vector>
#include <memory>

#include "VertexArray.h"

class VertexBufferLayout;

class VertexBuffer
{
    friend VertexArray;
private:
    uint32_t bufferID = 0;
    VertexArray va = VertexArray(0);

private:
public:
    VertexBuffer() = default;
    void Construct(const void* data, std::size_t size);
    inline VertexBuffer(const void* data, std::size_t size)
    {
        Construct(data, size);
    }
    ~VertexBuffer();

    inline VertexBuffer(VertexBuffer&& other) { this->~VertexBuffer();va = std::move(other.va);bufferID = other.bufferID;other.bufferID = 0; }
    inline void operator =(VertexBuffer&& other) { this->~VertexBuffer();va = std::move(other.va);bufferID = other.bufferID;other.bufferID = 0; }


    template<typename T>
    inline VertexBuffer(std::vector<T>& vec,const VertexBufferLayout& vl)
    {
        va = VertexArray();
        va.Bind();
        if (vec.size())
            Construct(&(vec[0]), vec.size() * sizeof(T));
        va.AddBuffer(*this,vl);
    }

    void Bind() const;
};