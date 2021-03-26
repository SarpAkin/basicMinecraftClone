#include "utility.hpp"

#include <pthread.h>

#include <iostream>
#include <string>
#include <thread>

std::string readFile(const char* file)
{
    std::fstream stream;
    stream.open(file, std::ios::in);
    if (stream.is_open())
        return std::string(std::istreambuf_iterator<char>(stream), {});
    else
    {
        std::cerr << "can't open the file with the name : " << file << '\n';
        return std::string();
    }
}

Vector2 rotateVectorInRadians(Vector2 v, float radian)
{
    Vector2 retV;
    retV.x = v.x * cos(radian) - v.y * sin(radian);
    retV.y = v.x * sin(radian) + v.y * cos(radian);
    return retV;
}

void SetThreadName(std::thread& t, const char* name)
{
#ifndef _WIN32
    if (std::string(name).length() > 16)
        std::cerr << "name is too big\n";
    pthread_setname_np(t.native_handle(), name);
#endif
}

void SetThreadName(const char* name)
{
#ifndef _WIN32
    if (std::string(name).length() > 16)
        std::cerr << "name is too big\n";
    std::cout << name << '\n';
    pthread_setname_np(pthread_self(), name);
#endif
}
