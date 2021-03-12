#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

#include "vectors.h"

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

//constexpr int log()