#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        class Torus : public ImplicitField
        {
        public:
            Torus() :
                mInner(1.0f),
                mOuter(3.0f),
                mCentre(0.0f)
            { }

            Torus(float inner, float outer, core::Point const& centre) :
                mInner(inner),
                mOuter(outer),
                mCentre(centre)
            { }

            ~Torus() = default;


            core::Normal grad(core::Point const& p) const override
            {
                using core::Normal;

                float sqrt = glm::length(p.xy() - mCentre.xy());
                float dx = 2.0f * (mInner - sqrt) * (p.x - mCentre.x) / sqrt;
                float dy = 2.0f * (mInner - sqrt) * (p.y - mCentre.y) / sqrt;
                float dz = 2.0f * (p.z - mCentre.z);

                return Normal(dx, dy, dz);
            }

            core::Point getSeed() const
            {
                return mCentre + mInner;
            }

        private:
            float sdf(core::Point const& p) const override
            {
                using core::Point;
                using core::Point2;

                float root = glm::length(p.xy() - mCentre.xy());
                float z = glm::length(p.z - mCentre.z);
                float left = (mInner - root) * (mInner - root);
                return left + z - (mOuter * mOuter);
            }

            core::BBox box() const override
            {
                using core::Point;

                Point p = {  mInner + mOuter,  mInner + mOuter, -mOuter };
                Point q = { -mInner - mOuter, -mInner - mOuter,  mOuter };
                return core::BBox(p, q);
            }

            float mInner, mOuter;
            core::Point mCentre;
        };
    }
}


#endif