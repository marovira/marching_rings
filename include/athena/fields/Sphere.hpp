#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_SPHERE_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_SPHERE_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        template <FilterFn filter>
        class Sphere : public ImplicitField<filter>
        {
        public:
            Sphere() :
                mRadius(1.0f),
                mCentre(0.0f)
            { }

            Sphere(float radius, core::Point const& centre) :
                mRadius(radius),
                mCentre(centre)
            { }

            ~Sphere() = default;

            core::BBox getBBox() const override
            {
                using core::BBox;
                using core::Point;

                return BBox(Point(-mRadius), Point(mRadius));
            }

            core::Normal grad(core::Point const& p) const override
            {
                return 2.0f * p;
            }

            core::Point getSeed() const
            {
                return mCentre + mRadius;
            }

        private:
            float sdf(core::Point const& p) const override
            {
                return glm::length(p) - mRadius;
            }

            float mRadius;
            core::Point mCentre;
        };
    }
}

#endif