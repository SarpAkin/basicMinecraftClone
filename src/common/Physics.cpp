#include "Physics.hpp"

#include <iostream>

bool AABBCheck(Transform& c0, const Transform& c1)
{
    // clang-format off
    return c0.pos.x < (c1.pos.x + c1.size.x) && c1.pos.x < (c0.pos.x + c0.size.x)  // check x axis
        && c0.pos.y < (c1.pos.y + c1.size.y) && c1.pos.y < (c0.pos.y + c0.size.y)  // check y axis
        && c0.pos.z < (c1.pos.z + c1.size.z) && c1.pos.z < (c0.pos.z + c0.size.z); // check z axis
    // clang-format on
}

void ResolveAABBCollision(Transform& current, const Transform& other)
{
    Vector3 cordDiff = current.GetMidPoint() - other.GetMidPoint();
    Vector3 sizeSum = (current.size + other.size) * .5f;
    Vector3 collisionDepth = sizeSum - Vector3(std::abs(cordDiff.x), std::abs(cordDiff.y), std::abs(cordDiff.z));

    Vector3 totalDisplacement = {0, 0, 0};

    // Find the smallest collision depth of all axises
    // And create a displacement for that.
    if (collisionDepth.x < collisionDepth.y)
    {
        if (collisionDepth.x < collisionDepth.z)
        {
            // x is the smallest
            if (cordDiff.x > 0)
                totalDisplacement.x = collisionDepth.x;
            else // if current objects x axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.x = -collisionDepth.x;
        }
        else
        {
            // z is the samllest
            if (cordDiff.z > 0)
                totalDisplacement.z = collisionDepth.z;
            else // if current objects z axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.z = -collisionDepth.z;
        }
    }
    else
    {
        if (collisionDepth.y < collisionDepth.z)
        {
            // y is the smallest
            if (cordDiff.y > 0)
                totalDisplacement.y = collisionDepth.y;
            else // if current objects y axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.y = -collisionDepth.y;
        }
        else
        {
            // z is the samllest
            if (cordDiff.z > 0)
                totalDisplacement.z = collisionDepth.z;
            else // if current objects z axis is smaller than others than displace it in thenegative axis.
                totalDisplacement.z = -collisionDepth.z;
        }
    }

    // Finaly displace the object
    current.pos += totalDisplacement;
}

void simulatePhysics(Transform& current, const std::vector<Transform>& others, float deltaT)
{
    // return if deltaT is zero to avoid crash
    if (deltaT == 0.0f)
        return;
    Vector3 startPos = current.pos;
    current.pos += current.velocity * deltaT;

    auto start_vel = current.velocity;

    for (auto& other : others)
    {
        if (AABBCheck(current, other))
        {
            ResolveAABBCollision(current, other);
        }
    }

    current.velocity = (current.pos - startPos) / deltaT;

    current.velocity.x = std::abs(current.velocity.x) < std::abs(start_vel.x) ? current.velocity.x : start_vel.x;
    current.velocity.y = std::abs(current.velocity.y) < std::abs(start_vel.y) ? current.velocity.y : start_vel.y;
    current.velocity.z = std::abs(current.velocity.z) < std::abs(start_vel.z) ? current.velocity.z : start_vel.z;
}

bool ChunkVSAABB(std::vector<std::shared_ptr<Entity>>::iterator e_it, float deltaT)
{
    auto& e = **e_it;
    const auto& c = *e.currentChunk;
    Transform& t = e.transform;
    Vector3 ChunkRealPos = Vector3(c.pos.x * chunk_size, 0, c.pos.y * chunk_size);
    Vector3 inChunkPos = t.pos;

    std::vector<Transform> blockColliders;
    blockColliders.reserve((int)((t.size.x + 1) * (t.size.y + 1) * (t.size.z + 1)));
    for (int x = std::floor(inChunkPos.x), x_end = std::ceil(inChunkPos.x + t.size.x) + 1; x < x_end; ++x)
        for (int y_end = std::floor(inChunkPos.y), y = std::ceil(inChunkPos.y + t.size.y); y >= y_end; --y)
            for (int z = std::floor(inChunkPos.z), z_end = std::ceil(inChunkPos.z + t.size.z) + 1; z < z_end; ++z)
            {
                if (c[{x, y, z}] != air)
                {
                    blockColliders.emplace_back(Vector3(x, y, z) + Vector3(-.5f, -.5f, -.5f), Vector3(1, 1, 1));
                }
            }
    auto prePos = t.pos;
    simulatePhysics(t, blockColliders, deltaT);
    bool retVal = t.pos != prePos;
    const int eRange = 2;
    if (t.pos.x < -eRange)
    {
        if (e.currentChunk->westernChunk)
        {
            t.pos.x += chunk_size;
            e.currentChunk->MoveEntity(e_it, *e.currentChunk->westernChunk);
        }
    }
    else if (t.pos.x > (eRange + chunk_size))
    {
        if (e.currentChunk->easternChunk)
        {
            t.pos.x -= chunk_size;
            e.currentChunk->MoveEntity(e_it, *e.currentChunk->easternChunk);
        }
    }
    else if (t.pos.z < -eRange)
    {
        if (e.currentChunk->southernChunk)
        {
            t.pos.z += chunk_size;
            e.currentChunk->MoveEntity(e_it, *e.currentChunk->southernChunk);
        }
    }
    else if (t.pos.z > (eRange + chunk_size))
    {
        if (e.currentChunk->northernChunk)
        {
            t.pos.z -= chunk_size;
            e.currentChunk->MoveEntity(e_it, *e.currentChunk->northernChunk);
        }
    }
    return retVal;
}
