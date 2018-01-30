#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_LATTICE_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_LATTICE_HPP
#pragma once

#include "Polygonizer.hpp"
#include "Voxel.hpp"

#include <atlas/math/Math.hpp>
#include <vector>

namespace athena
{
    namespace polygonizer
    {
        struct Lattice
        {
            Lattice() = default;

            void makeLattice(std::vector<Voxel> const& voxels);

            std::vector<atlas::math::Point> vertices;
            std::vector<std::uint32_t> indices;
        };
    }
}

#endif