#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_CYLINDER_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_CYLINDER_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        class Box : public ImplicitField
        {
        public:
            Box()
            { }

            atlas::math::Normal grad(atlas::math::Point const& p) const override
            {
                return atlas::math::Normal();
            }

            std::vector<atlas::math::Point> getSeeds(atlas::math::Normal const& u,
                float offset) const override
            {
                return {};
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                return 0.0f;
            }

            atlas::utils::BBox box() const override
            {
                return atlas::utils::BBox();
            }

        };
    }
}

#endif