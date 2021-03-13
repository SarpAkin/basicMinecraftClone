#include "opengl_.hpp"

void clearGLErrors()
{
    while (glGetError());
}

bool logGLCalls()
{
    bool ret = true;
    while (GLenum error = glGetError())
    {
        std::cout << std::hex << std::uppercase;
        std::cout << "[GL error] " << error << "\n";
        std::cout << std::dec << std::nouppercase;
        ret = false;
    }
    return ret;
}
