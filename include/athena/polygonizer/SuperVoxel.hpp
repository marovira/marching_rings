#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_SUPER_VOXEL_HPP
#define ATHENA_INLCUDE_ATHENA_POLYGONIZER_SUPER_VOXEL_HPP

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
                float value = 0.0f;
                for (auto& field : fields)
                {
                    value += field->eval(p);
                }

                return value;
            }

            atlas::math::Normal grad(atlas::math::Point const& p) const
            {
                atlas::math::Normal gradient;
                for (auto& field : fields)
                {
                    gradient += field->grad(p);
                }

                return gradient;
            }

            glm::u32vec2 id;
            std::vector<fields::ImplicitFieldPtr> fields;
            atlas::utils::BBox cell;
        };
    }
}

#endif