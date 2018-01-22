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

            Sphere(float radius, atlas::math::Point const& centre) :
                mRadius(radius),
                mCentre(centre)
            { }

            ~Sphere() = default;


            atlas::math::Normal grad(atlas::math::Point const& p) const override
            {
                return 2.0f * (p - mCentre);
            }

            atlas::math::Point getSeed() const
            {
                return mCentre + mRadius;
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                return glm::length(p - mCentre) - mRadius;
            }

            atlas::utils::BBox box() const override
            {
                using atlas::utils::BBox;
                using atlas::math::Point;

                return BBox(Point(-mRadius), Point(mRadius));
            }

            float mRadius;
            atlas::math::Point mCentre;
        };
    }
}

#endif