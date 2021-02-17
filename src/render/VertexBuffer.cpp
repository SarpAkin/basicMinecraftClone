#include "VertexBuffer.h"

#include "opengl_.h"



VertexBuffer::VertexBuffer(const void* data, std::size_t size)
{
    GLCALL(glGenBuffers(1, &bufferID));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, bufferID));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
    if (bufferID)
        GLCALL(glDeleteBuffers(1, &bufferID));
}

void VertexBuffer::Bind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, bufferID));
}