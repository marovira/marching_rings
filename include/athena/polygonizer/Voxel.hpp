#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_VOXEL_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_VOXEL_HPP

#pragma once

#include <atlas/math/Math.hpp>

#include <cinttypes>
#include <array>

namespace athena
{
    namespace polygonizer
    {
        struct VoxelPoint
        {
            VoxelPoint() :
                value(0.0f, 0.0f, 0.0f, -1.0f),
                g(0.0f)
            { }

            VoxelPoint(atlas::math::Point const& p, float v) :
                value(p, v),
                g(0.0f)
            { }

            VoxelPoint(atlas::math::Point const& p, float v, 
                atlas::math::Normal const& grad) :
                value(p, v),
                g(grad)
            { }

            atlas::math::Point4 value;
            atlas::math::Normal g;
        };

        constexpr auto invalidUint()
        {
            return std::numeric_limits<std::uint32_t>::max();
        }


        using PointId = glm::u32vec2;
        using VoxelId = PointId;

        struct Voxel
        {
            Voxel() :
                id(invalidUint())
            { }

            Voxel(glm::u64vec2 const& p) :
                id(p)
            { }

            bool isValid() const
            {
                glm::u64vec2 invalid(invalidUint());
                return (
                    id.x != invalid.x &&
                    id.y != invalid.y);
            }

            std::array<VoxelPoint, 4> points;
            VoxelId id;
        };
    }
}

#endif