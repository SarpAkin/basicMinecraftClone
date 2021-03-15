#include "utility.hpp"

#include <pthread.h>

#include <string>
#include <thread>
#include <iostream>

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

void SetThreadName(std::thread& t,const char* name)
{
    if(std::string(name).length() > 16)
        std::cerr << "name is too big\n";
    pthread_setname_np(t.native_handle(),name);
}

void SetThreadName(const char* name)
{
    if(std::string(name).length() > 16)
        std::cerr << "name is too big\n";
    std::cout << name << '\n';
    pthread_setname_np(pthread_self(),name);
}
