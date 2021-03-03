#pragma once

#include <iostream>
#include <ctime>

#include <PerlinNoise.hpp>

#include <glm/vec3.hpp>

//typedef siv::BasicPerlinNoise<double> PerlinNoise;


/*Wrpaer class around siv::PerlinNoise*/
class PerlinNoise
{
public:
    siv::BasicPerlinNoise<double> noise;
public:

    PerlinNoise()
    : noise(time(NULL))
    {
        std::cout << time(NULL) << '\n';
    }

    template<typename T>
    inline float operator[](glm::vec<3, T> vec) const
    {
        return noise.noise3D(vec.x, vec.y, vec.z);
    }

    template<typename T>
    inline float operator[](glm::vec<2, T> vec) const
    {
        return noise.noise3D(vec.x, vec.y,0);
    }

    template<typename T>
    inline float operator[](glm::vec<1, T> vec) const
    {
        return noise.noise3D(vec.x,0,0);
    }
};