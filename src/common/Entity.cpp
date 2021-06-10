#include "Entity.hpp"

#include <assert.h>
#include <cstdint>
#include <vector>

#include "chunk.hpp"
#include "game.hpp"
#include "utility.hpp"
#include "vectors.hpp"

// CPP
Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::Serialize(Message& m)
{
    m.push_back(entityID);
    m.push_back(transform);
    assert(currentChunk); // current chunk can't be null when serializing
    m.push_back(currentChunk->pos);
}

bool Entity::Deserialize(Message& m, Game* g)
{
    m.pop_front(entityID);
    m.pop_front(transform);
    currentChunk = g->chunks[m.pop_front<Vector2Int>()].get();
    // assert(currentChunk);//current chunk can't be null after deserializing
    return (bool)currentChunk;
}

#ifndef SERVER_SIDE

#include "../client/render/renderer.hpp"
#include "../client/render/shader.hpp"

Shader EntityShader;

void Entity::StaticInit()
{
    EntityShader = Shader(readFile("res/shaders/Entity_vertex.glsl"), readFile("res/shaders/Entity_fragment.glsl"));
}

void Entity::InitMesh()
{

    float x_b_p = transform.size.x;
    float x_b_n = 0;

    float y_b_p = transform.size.y;
    float y_b_n = 0;

    float z_b_p = transform.size.z;
    float z_b_n = 0;

    // clang-format off
    std::vector<Vector3> verticies = 
    {
        //y+
        Vector3(x_b_p, y_b_p, z_b_p),//0
        Vector3(x_b_p, y_b_p, z_b_n),//1
        Vector3(x_b_n, y_b_p, z_b_n),//2
        Vector3(x_b_n, y_b_p, z_b_p),//3
        //y-
        Vector3(x_b_p, y_b_n, z_b_p),//0
        Vector3(x_b_n, y_b_n, z_b_p),//3
        Vector3(x_b_n, y_b_n, z_b_n),//2
        Vector3(x_b_p, y_b_n, z_b_n),//1

        //x+
        Vector3(x_b_p, y_b_p, z_b_p),//0
        Vector3(x_b_p, y_b_n, z_b_p),//3
        Vector3(x_b_p, y_b_n, z_b_n),//2
        Vector3(x_b_p, y_b_p, z_b_n),//1
        //x-
        Vector3(x_b_n, y_b_p, z_b_p),//0
        Vector3(x_b_n, y_b_p, z_b_n),//1
        Vector3(x_b_n, y_b_n, z_b_n),//2
        Vector3(x_b_n, y_b_n, z_b_p),//3

        //z+
        Vector3(x_b_p, y_b_p, z_b_p),//0
        Vector3(x_b_n, y_b_p, z_b_p),//3
        Vector3(x_b_n, y_b_n, z_b_p),//2
        Vector3(x_b_p, y_b_n, z_b_p),//1
        //z-
        Vector3(x_b_p, y_b_p, z_b_n),//0
        Vector3(x_b_p, y_b_n, z_b_n),//1
        Vector3(x_b_n, y_b_n, z_b_n),//2
        Vector3(x_b_n, y_b_p, z_b_n),//3

    };
    // clang-format on

    std::vector<std::uint16_t> indicies;
    indicies.reserve(36);
    for (int i = 0; i < 6; ++i)
    {
        uint16_t offs = i * 4;
        // indicies.insert(indicies.end(), {0, 1, 2, /**/ 0, 2, 3});
        indicies.push_back(0 + offs);
        indicies.push_back(1 + offs);
        indicies.push_back(2 + offs);

        indicies.push_back(0 + offs);
        indicies.push_back(2 + offs);
        indicies.push_back(3 + offs);
    }

    VertexBufferLayout vl;
    vl.Push<float>(3);

    vBuf = VertexBuffer(verticies, vl);
    iBuf = IndexBuffer(indicies);

    isMeshinitilized = true;
}

void Entity::Draw(glm::mat4x4 mv, Renderer& r)
{
    if (!isMeshinitilized)
    {
        InitMesh();
    }
    EntityShader.Bind();
    EntityShader.SetUniformMat4("u_MVP", mv);
    r.DrawU16(vBuf.va, iBuf, EntityShader);
}
#endif