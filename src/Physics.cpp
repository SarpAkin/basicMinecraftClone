#include "Physics.h"

#include <iostream>

bool AABBCheck(Transform& c0, const Transform& c1)
{
    return c0.pos.x < c1.pos.x + c1.size.x && c1.pos.x < c0.pos.x + c0.size.x //check x axis
        && c0.pos.y < c1.pos.y + c1.size.y && c1.pos.y < c0.pos.y + c0.size.y //check y axis
        && c0.pos.z < c1.pos.z + c1.size.z && c1.pos.z < c0.pos.z + c0.size.z;//check z axis
}

void ResolveAABBCollision(Transform& current, const Transform& other)
{
    Vector3 cordDiff = current.GetMidPoint() - other.GetMidPoint();
    Vector3 sizeSum = (current.size + other.size) * .5f;
    Vector3 collisionDepth = sizeSum - Vector3(std::abs(cordDiff.x), std::abs(cordDiff.y), std::abs(cordDiff.z));

    Vector3 totalDisplacement = { 0,0,0 };

    //Find the smallest collision depth of all axises 
    //And create a displacement for that.
    if (collisionDepth.x < collisionDepth.y)
    {
        if (collisionDepth.x < collisionDepth.z)
        {
            //x is the smallest
            if (cordDiff.x > 0)
                totalDisplacement.x = collisionDepth.x;
            else//if current objects x axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.x = -collisionDepth.x;
        }
        else
        {
            //z is the samllest
            if (cordDiff.z > 0)
                totalDisplacement.z = collisionDepth.z;
            else//if current objects z axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.z = -collisionDepth.z;
        }
    }
    else
    {
        if (collisionDepth.y < collisionDepth.z)
        {
            //y is the smallest
            if (cordDiff.y > 0)
                totalDisplacement.y = collisionDepth.y;
            else//if current objects y axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.y = -collisionDepth.y;
        }
        else
        {
            //z is the samllest
            if (cordDiff.z > 0)
                totalDisplacement.z = collisionDepth.z;
            else//if current objects z axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.z = -collisionDepth.z;
        }
    }

    //Finaly displace the object
    current.pos += totalDisplacement;
}

void simulatePhysics(Transform& current, const std::vector<Transform>& others, float deltaT)
{
    //return if deltaT is zero to avoid crash
    if (deltaT == 0.0f)
        return;
    Vector3 startPos = current.pos;
    current.pos += current.velocity * deltaT;
    for (auto& other : others)
    {
        if (AABBCheck(current, other))
        {
            ResolveAABBCollision(current, other);
        }
    }
    current.velocity = (current.pos - startPos) / deltaT;
}

void ChunkVSAABB(Transform& t, const Chunk& c, float deltaT)
{
    Vector3 ChunkRealPos = Vector3(c.pos.x * chunk_size,0,c.pos.y * chunk_size);
    Vector3 inChunkPos = t.pos - ChunkRealPos;


    std::vector<Transform> blockColliders;
    blockColliders.reserve((int)((t.size.x + 1)*(t.size.y + 1)*(t.size.z + 1)));
    for (int x = std::floor(inChunkPos.x), x_end = std::ceil(inChunkPos.x + t.size.x) + 1;x < x_end;++x)
        for (int y = std::floor(inChunkPos.y), y_end = std::ceil(inChunkPos.y + t.size.y) + 1;y < y_end;++y)
            for (int z = std::floor(inChunkPos.z), z_end = std::ceil(inChunkPos.z + t.size.z) + 1;z < z_end;++z)
            {
                if (c[{x, y, z}] != air)
                {
                    blockColliders.emplace_back(Vector3(x, y, z) + Vector3(-.5f,-.5f,-.5f), Vector3(1, 1, 1));
                }
            }
    
    Vector3 Normalpos = t.pos;
    t.pos = inChunkPos;
    simulatePhysics(t, blockColliders, deltaT);
    t.pos = Normalpos + (t.pos - inChunkPos);
}