#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_SPHERE_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_SPHERE_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        class Sphere : public ImplicitField
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


            core::Normal grad(core::Point const& p) const override
            {
                return 2.0f * (p - mCentre);
            }

            core::Point getSeed() const
            {
                return mCentre + mRadius;
            }

        private:
            float sdf(core::Point const& p) const override
            {
                return glm::length(p - mCentre) - mRadius;
            }

            core::BBox box() const override
            {
                using core::BBox;
                using core::Point;

                return BBox(Point(-mRadius), Point(mRadius));
            }

            float mRadius;
            core::Point mCentre;
        };
    }
}

#endif