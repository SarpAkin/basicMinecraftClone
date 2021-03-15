#include "Entity.hpp"

#include <assert.h>

#include "chunk.hpp"
#include "game.hpp"
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
    assert(currentChunk);//current chunk can't be null when serializing
    m.push_back(currentChunk->pos);
    
}

bool Entity::Deserialize(Message& m,Game* g)
{
    m.pop_front(entityID);
    m.pop_front(transform);
    currentChunk = g->chunks[m.pop_front<Vector2Int>()].get();
    //assert(currentChunk);//current chunk can't be null after deserializing
    return (bool)currentChunk;
}