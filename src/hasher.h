#pragma once

#include <utility>
#include <string>

template<typename T>
struct Hasher
{
    static std::hash<std::string> h;
    inline std::size_t operator()(const T& k) const
    {
        return h(std::string((char*)&k, (char*)&k + sizeof(T)));
    }
};

template<typename T>
std::hash<std::string> Hasher<T>::h;

template<typename T>
struct Equal
{
    static Hasher<T> h;
    inline bool operator()(const T& k0, const T& k1) const
    {
        return h(k0) == h(k1);
    }
};

template<typename T>
Hasher<T> Equal<T>::h;