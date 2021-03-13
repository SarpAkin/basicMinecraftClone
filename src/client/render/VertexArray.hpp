#pragma once

#include <ostream>

class VertexBuffer;
class VertexBufferLayout;

class VertexArray
{
private:
    uint32_t arrayID = 0;
private:
public:
    /*Use this when you wan't to create a vertex array before initilazing opengl*/
    inline VertexArray(int noInit) {};
    VertexArray();
    ~VertexArray();

    inline VertexArray(VertexArray&& other) { arrayID = other.arrayID;other.arrayID = 0; }
    inline void operator =(VertexArray&& other) {
        this->~VertexArray();
        arrayID = other.arrayID;other.arrayID = 0;
    }

    void Bind() const;
    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
};
