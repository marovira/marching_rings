#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP

#pragma once

#include "Fields.hpp"
#include "Filters.hpp"

#include <atlas/math/Math.hpp>
#include <atlas/utils/BBox.hpp>

#include <vector>

namespace athena
{
    namespace fields
    {
        class ImplicitField
        {
        public:
            ImplicitField()
            { }

            virtual ~ImplicitField() = default;

            atlas::utils::BBox getBBox() const
            {
                auto b = box();
                //b.expand(1.0f);
                return b;
            }

            float eval(atlas::math::Point const& p) const
            {
                return global(sdf(p));
            }

            virtual atlas::math::Normal grad(
                atlas::math::Point const& p) const = 0;
            virtual std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const = 0;

        protected:
            virtual float sdf(atlas::math::Point const& p) const = 0;
            virtual atlas::utils::BBox box() const = 0;
        };
    }
}


#endif