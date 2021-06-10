#include "VertexArray.hpp"

#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"
#include "opengl_.hpp"

VertexArray::VertexArray()
{
    GLCALL(glGenVertexArrays(1, &arrayID));
}

VertexArray::~VertexArray()
{
    if (arrayID)
    {
        GLCALL(glDeleteVertexArrays(1, &arrayID));
    }
}

void VertexArray::Bind() const
{
    GLCALL(glBindVertexArray(arrayID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
    Bind();
    vb.Bind();
    const auto& elements = layout.getElements();
    uint32_t offset = 0;
    for (uint32_t i = 0; i < elements.size(); ++i)
    {
        const auto& element = elements[i];
        GLCALL(glEnableVertexAttribArray(i));

        if (element.type >= GL_BYTE && element.type <= GL_UNSIGNED_INT)
        {
            GLCALL(glVertexAttribIPointer(i, element.count, element.type, layout.getStride(),
                (const void*)(size_t)offset));
        }
        else
        {
            GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(),
                (const void*)(size_t)offset));
        }
        offset += element.size;
    }
}
