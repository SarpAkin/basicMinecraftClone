#pragma once

#include <cstdio>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <thread>
#include <type_traits>

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

template <typename T, typename indexer = uint32_t>
class vectorBasedDic
{
    static constexpr bool is_ptr = requires(T t)
    {
        t = nullptr;
    };
    static constexpr bool is_weak_ptr = requires(T t)
    {
        t.lock() = nullptr; // for weak pointers
    };
    static_assert(is_ptr || is_weak_ptr, "not a pointer!");

    std::vector<indexer> deletedElementPositions;

public:
    std::vector<T> items;

    indexer push_pack(T t)
    {
        if (deletedElementPositions.size())
        {
            indexer index = deletedElementPositions.back();
            deletedElementPositions.pop_back();
            items[index] = t;
            return index;
        }
        else
        {
            items.push_back(t);
            return items.size() - 1;
        }
    }

    void erase(indexer i)
    {
        if constexpr (is_weak_ptr)
            items[i].reset();
        else
            items[i] = nullptr;
        deletedElementPositions.push_back(i);
    }

    inline auto operator[](indexer i)
    {
        if (i < items.size())
            return items[i];
        else
            return T();
    }
};

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

#define MEASURE_TIME(func)                                                                                             \
    auto s = std::chrono::steady_clock::now();                                                                         \
    func;                                                                                                              \
    std::cout << "time elapsed :" << ((double)(std::chrono::steady_clock::now() - s).count()) / 1000000                \
              << " milliseconds" << std::endl;

#define GEN_SERIALIZATION_FUNCTIONS(fields...)                                                                         \
    inline void Serialize(Message& m) const                                                                            \
    {                                                                                                                  \
        SerializeMultiple(m, fields);                                                                                  \
    }                                                                                                                  \
    inline void Deserialize(Message& m)                                                                                \
    {                                                                                                                  \
        DeserializeMultiple(m, fields);                                                                                \
    }
