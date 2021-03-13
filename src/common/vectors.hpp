#pragma once

#include <cmath>
#include <inttypes.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

typedef glm::mat4 Mat4x4;

typedef glm::vec<2,float> Vector2;
typedef glm::vec<3,float> Vector3;
typedef glm::vec<4,float> Vector4;

typedef glm::vec<2,double> Vector2d;
typedef glm::vec<3,double> Vector3d;
typedef glm::vec<4,double> Vector4d;

typedef glm::vec<2,int32_t> Vector2Int;
typedef glm::vec<3,int32_t> Vector3Int;
typedef glm::vec<4,int32_t> Vector4Int;

typedef glm::vec<2,int64_t> Vector2Long;
typedef glm::vec<3,int64_t> Vector3Long;
typedef glm::vec<4,int64_t> Vector4Long;

template<typename Float>
inline Vector3Int FloorVec(glm::vec<3,Float> v)
{
    return Vector3Int(std::floor(v.x),std::floor(v.y),std::floor(v.z));
}

template<typename Float>
inline Vector2Int FloorVec(glm::vec<2,Float> v)
{
    return Vector2Int(std::floor(v.x),std::floor(v.y));
}

template<typename Float,int N>
inline glm::vec<N,Float> ModVec(glm::vec<N,Float> vec,float mod)
{
    glm::vec<N,Float> retVec;
    for(int i = 0;i < N;++i)
        retVec[i] = std::fmod(vec[i]);
    return retVec;
}
