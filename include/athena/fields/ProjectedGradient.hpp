#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_PROJECTED_GRADIENT_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_PROJECTED_GRADIENT_HPP

#pragma once

#include "ImplicitField.hpp"

#include <atlas/core/Macros.hpp>

namespace athena
{
    namespace fields
    {
        class ProjectedGradient : public ImplicitField
        {
        public:
            ProjectedGradient(atlas::math::Normal const& n) :
                ImplicitField(),
                mNpi(glm::normalize(n)),
                mNpiDotNpi(1.0f)
            { }

            ~ProjectedGradient() = default;

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const override
            {
                UNUSED(u);
                return {};
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                return glm::length(p - glm::proj(p, mNpi));
            }

            atlas::math::Normal sdg(atlas::math::Point const& p) const override
            {
                using atlas::math::Normal;

                auto const& n = mNpi;
                auto const& g = p;

                const float nDotG = glm::dot(n, g);
                const float denominator = 2 * glm::length(g - glm::dot(g, n) * n);

                float x =
                    (2 * (1 - n.x * n.x) * (g.x - n.x * (nDotG))) -
                    (2 * (n.x * n.y)     * (g.y - n.y * (nDotG))) -
                    (2 * (n.x * n.z)     * (g.z - n.z * (nDotG)));

                float y =
                    (2 * (1 - n.y * n.y) * (g.y - n.y * (nDotG))) -
                    (2 * (n.x * n.y)     * (g.x - n.x * (nDotG))) -
                    (2 * (n.y * n.z)     * (g.z - n.z * (nDotG)));

                float z =
                    (2 * (1 - n.z * n.z) * (g.z - n.z * (nDotG))) -
                    (2 * (n.x * n.z)     * (g.x - n.x * (nDotG))) -
                    (2 * (n.y * n.z)     * (g.y - n.y * (nDotG)));

                Normal grad{ x, y, z };
                grad /= denominator;

                return grad;
            }

            atlas::utils::BBox box() const override
            {
                return atlas::utils::BBox();
            }

            atlas::math::Normal mNpi;
            float mNpiDotNpi;
        };
    }

}

#endif