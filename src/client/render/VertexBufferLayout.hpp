#pragma once

#include <ostream>
#include <vector>

#include "opengl_.hpp"

struct VertexBufferElement
{
    uint32_t type;
    uint32_t count;
    uint32_t size;
    uint32_t normalized;
};

template<typename T>
inline void Helper_Push_Func(uint32_t count, std::vector<VertexBufferElement>& elements, uint32_t& stride)
{
    assert(0);
    //static_assert(false, "invalid type!\n");
}

template<>
inline void Helper_Push_Func<float>(uint32_t count, std::vector<VertexBufferElement>& elements, uint32_t& stride)
{
    elements.push_back({ GL_FLOAT,count,static_cast<uint32_t>(sizeof(float)) * count,false });
    stride += sizeof(float) * count;
}

template<>
inline void Helper_Push_Func<uint32_t>(uint32_t count, std::vector<VertexBufferElement>& elements, uint32_t& stride)
{
    elements.push_back({ GL_UNSIGNED_INT,count,static_cast<uint32_t>(sizeof(uint32_t)) * count,false });
    stride += sizeof(uint32_t) * count;
}

class VertexBufferLayout
{
private:
    std::vector<VertexBufferElement> elements;
    uint32_t stride = 0;
public:

    template<typename T>
    void Push(uint32_t count)
    {
        Helper_Push_Func<T>(count,elements,stride);
    }

    inline const std::vector<VertexBufferElement>& getElements() const { return elements; }
    inline uint32_t getStride() const { return stride; }
};

