#ifndef ATHENA_INCLUDE_ATHENA_BLOB_VOXEL_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_VOXEL_HPP

#pragma once

#include "athena/core/Math.hpp"

#include <cinttypes>
#include <array>

namespace athena
{
    namespace blob
    {
        struct VoxelPoint
        {
            VoxelPoint() :
                value(0.0f, 0.0f, 0.0f, -1.0f),
                g(0.0f)
            { }

            VoxelPoint(core::Point const& p, float v) :
                value(p, v),
                g(0.0f)
            { }

            VoxelPoint(core::Point const& p, float v, core::Normal const& grad) :
                value(p, v),
                g(grad)
            { }

            core::Point4 value;
            core::Normal g;
        };

        template <typename T>
        constexpr auto invalidUint()
        {
            return std::numeric_limits<T>::max();
        }

        constexpr auto invalidUint64()
        {
            return invalidUint<std::uint64_t>();
        }

        using PointId = glm::u64vec3;
        using VoxelId = PointId;

        struct Voxel
        {
            Voxel() :
                id(invalidUint64())
            { }

            Voxel(glm::u64vec3 const& p) :
                id(p)
            { }

            bool isValid() const
            {
                glm::u64vec3 invalid(invalidUint64());
                return (
                    id.x != invalid.x &&
                    id.y != invalid.y && 
                    id.z != invalid.z);
            }

            std::array<VoxelPoint, 8> points;
            VoxelId id;
        };
    }
}

#endif