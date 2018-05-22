#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_CONTOUR_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_CONTOUR_HPP

#pragma once

#include "Polygonizer.hpp"
#include "Voxel.hpp"

#include <atlas/math/Math.hpp>
#include <vector>

namespace athena
{
    namespace polygonizer
    {
        struct Contour
        {
            Contour() = default;

            void makeContour(
                std::vector<std::vector<std::vector<FieldPoint>>> const& contours);
            void clearBuffers();

            std::vector<atlas::math::Point> vertices;
            std::vector<std::uint32_t> indices;
            std::vector<std::pair<std::uint32_t, std::uint32_t>> indexOffsets;
            std::vector<std::pair<std::uint32_t, std::uint32_t>> vertexOffsets;
        };
    }
}

#endif