#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_SUPER_VOXEL_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_SUPER_VOXEL_HPP

#pragma once

#include "athena/fields/ImplicitField.hpp"

#include <atlas/math/Math.hpp>
#include <atlas/utils/BBox.hpp>
#include <vector>

namespace athena
{
    namespace polygonizer
    {
        struct SuperVoxel
        {
            SuperVoxel()
            { }

            float eval(atlas::math::Point const& p) const
            {
                return field->eval(p);
            }

            atlas::math::Normal grad(atlas::math::Point const& p) const
            {
                return field->grad(p);
            }

            glm::u32vec2 id;
            fields::ImplicitFieldPtr field;
            atlas::utils::BBox cell;
        };
    }
}

#endif