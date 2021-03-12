#pragma once

#include <vector>
#include <ostream>

class IndexBuffer
{
private:
    uint32_t bufferID = 0;
public:
    uint32_t size = 0;
private:
public:
    IndexBuffer() = default;
    void Construct(const void* data, std::size_t size);
    inline IndexBuffer(const void* data, std::size_t size)
    {
        Construct(data, size);
    }
    ~IndexBuffer();

    template<typename T>
    inline IndexBuffer(std::vector<T>& vec)
    {
        if(vec.size())
        Construct(&(vec[0]), vec.size() * sizeof(T));
    }

    inline IndexBuffer(IndexBuffer&& other) { this->~IndexBuffer();size = other.size;bufferID = other.bufferID;other.bufferID = 0; }
    inline void operator =(IndexBuffer&& other) { this->~IndexBuffer();size = other.size;bufferID = other.bufferID;other.bufferID = 0; }
    

    void Bind() const;
};
