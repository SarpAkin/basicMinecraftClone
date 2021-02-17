#include "IndexBuffer.h"

#include "opengl_.h"



IndexBuffer::IndexBuffer(const void* data, std::size_t size_)
: size(size_)
{
    GLCALL(glGenBuffers(1, &bufferID));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
    if(bufferID)
        GLCALL(glDeleteBuffers(1,&bufferID));
}

void IndexBuffer::Bind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferID));
}