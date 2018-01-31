#ifndef ATHENA_INCLUDE_ATHENA_BLOB_TABLES_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_TABLES_HPP

#pragma once

#include <atlas/math/Math.hpp>

#include <vector>
#include <map>

namespace athena
{
    namespace polygonizer
    {
        const std::vector<glm::u32vec2> VoxelDecals = 
        {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };

        const std::map<int, glm::ivec2> EdgeDecals = 
        {
            {0, {0, -1}},
            {1, {1, 0}},
            {2, {0, 1}},
            {3, {-1, 0}}
        };
    }
}

#endif