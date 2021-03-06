#pragma once

#include <vector>

#include "vectors.h"
#include "Entity.h"
#include "chunk.h"

inline void applyDrag(Transform& t,float DeltaT)
{
    t.velocity -= t.velocity * (t.drag * DeltaT);
}

bool AABBCheck(Transform& c0,const Transform& c1);

//Only Moves current. other stays still
void ResolveAABBCollision(Transform& current, const Transform& other);

void simulatePhysics(Transform& current,const std::vector<Transform>& others, float deltaT);

void ChunkVSAABB(Transform& t,const Chunk& c, float deltaT);