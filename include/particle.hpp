#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <HSGIL/math/vec3.hpp>

struct Particle
{
    gil::Vec3f v;
    gil::Vec3f r;
    gil::Vec3f f;

    float density;
    float pressure;
    gil::Vec3f color;
};

#endif // PARTICLE_HPP
