#pragma once

#include <cstdio>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <thread>

#include "net/message.hpp"

#include "vectors.hpp"

std::string readFile(const char* file);

inline std::string readFile(const std::string& file)
{
    return readFile(file.c_str());
}

Vector2 rotateVectorInRadians(Vector2 v, float radian);

inline Vector2 rotateVectorIndegrees(Vector2 v, float radian)
{
    return rotateVectorInRadians(v, glm::radians(radian));
}

void SetThreadName(std::thread& t, const char* name);
inline void SetThreadName(std::thread& t, std::string name)
{
    SetThreadName(t, name.c_str());
}

void SetThreadName(const char* name);
inline void SetThreadName(std::string name)
{
    SetThreadName(name.c_str());
}
// constexpr int log()

template <typename T, typename... Types>
void SerializeMultiple(Message& m, T& arg0)
{
    m.push_back(arg0);
}

template <typename T, typename... Types>
void SerializeMultiple(Message& m, T& arg0, Types&... args)
{
    m.push_back(arg0);
    SerializeMultiple(m, args...);
}

template <typename T, typename... Types>
void DeserializeMultiple(Message& m, T& arg0)
{
    m.pop_front(arg0);
}

template <typename T, typename... Types>
void DeserializeMultiple(Message& m, T& arg0, Types&... args)
{
    m.pop_front(arg0);
    DeserializeMultiple(m, args...);
}

#define GEN_SERIALIZATION_FUNCTIONS(fields...)                                                                         \
    inline void Serialize(Message& m) const                                                                            \
    {                                                                                                                  \
        SerializeMultiple(m, fields);                                                                                  \
    }                                                                                                                  \
    inline void Deserialize(Message& m)                                                                                \
    {                                                                                                                  \
        DeserializeMultiple(m, fields);                                                                                \
    }
