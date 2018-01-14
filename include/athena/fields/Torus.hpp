#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        template <typename FilterFn filter>
        class Torus : public ImplicitField<filter>
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

            core::BBox getBBox() const override
            {
                return core::BBox();
            }

            core::Normal grad(core::Point const& p) const override
            {
                // TODO: Fill me in!
                using core::Normal;
                return Normal(0.0f);
            }

            core::Point getSeed() const
            {
                // TODO: Fill me in!
                return core::Point(0.0f);
            }

        private:
            float sdf(core::Point const& p) const override
            {
                // TODO: Fill me in!
                return 0.0f;
            }

            float mInner, mOuter;
            core::Point mCentre;
        };
    }
}


#endif