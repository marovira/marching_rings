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
                using atlas::math::Point;

                return Point();
            }

            atlas::utils::BBox box() const override
            {
                return atlas::utils::BBox();
            }

            atlas::math::Normal mNpi;
            float mNpiDotNpi
        };
    }

}

#endif