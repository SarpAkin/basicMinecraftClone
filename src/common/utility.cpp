#include "utility.hpp"

std::string readFile(const char* file)
{
    std::fstream stream;
    stream.open(file, std::ios::in);
    if (stream.is_open())
        return std::string(std::istreambuf_iterator<char>(stream), {});
    else
        return std::string();
}


Vector2 rotateVectorInRadians(Vector2 v, float radian)
{
    Vector2 retV;
    retV.x = v.x * cos(radian) - v.y * sin(radian);
    retV.y = v.x * sin(radian) + v.y * cos(radian);
    return retV;
}
