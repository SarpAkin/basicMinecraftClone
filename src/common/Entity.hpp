#pragma once

#include <inttypes.h>

#include "vectors.hpp"

typedef uint32_t EntityID;

struct Transform
{
    Vector3 pos;
    Vector3 size;
    Vector3 velocity;
    float drag = 0.4;
    inline Vector3 GetMidPoint() const
    {
        return pos + size * .5f;
    }
    inline void SetMidPoint(Vector3 v)
    {
        pos = v - (size * .05f);
    }
    Transform() = default;
    inline Transform(Vector3 pos_,Vector3 size_)
    {
        pos = pos_;
        size = size_;
    }
};

class Chunk;

class Entity
{
private:

public:
    EntityID entityID;
    Chunk* currentChunk = nullptr;
    Transform transform;
private://func
public:
    Entity();
    ~Entity();

};
