#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <inttypes.h>

#include "utility.hpp"
#include "vectors.hpp"

#ifndef SERVER_SIDE

#include "../client/render/IndexBuffer.hpp"
#include "../client/render/VertexBuffer.hpp"
#include "../client/render/VertexBufferLayout.hpp"

#endif

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
    inline Transform(Vector3 pos_, Vector3 size_)
    {
        pos = pos_;
        size = size_;
    }
};

class Renderer;
class Game;
class Chunk;

class Entity
{
private:
#ifndef SERVER_SIDE
    VertexBuffer vBuf;
    IndexBuffer iBuf;
#endif

public:
    EntityID entityID;
    Chunk* currentChunk = nullptr;
    Transform transform;
    #ifndef SERVER_SIDE
    bool isVisible = true;
    bool isMeshinitilized = false;
    #endif
private: // func
public:
    void Serialize(Message& m);
    bool Deserialize(Message& m, Game* g);

    Entity();
    ~Entity();

#ifndef SERVER_SIDE
    void InitMesh();
    void Draw(glm::mat4x4 mv, Renderer& r);
    
    static void StaticInit();
#endif
};
