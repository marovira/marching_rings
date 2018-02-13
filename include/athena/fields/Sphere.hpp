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

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const override
            {
                // First project the centre onto the plane.
                auto v = mCentre - u;
                auto d = glm::proj(v, glm::normalize(u));
                auto proj = mCentre - d;

                float radius = mRadius;
                float l = glm::length(proj - mCentre);
                if (l > radius)
                {
                    return {};
                }

                float rp = 
                    glm::sqrt((radius * radius) - glm::length2(proj - mCentre));
                auto seed = proj;
                for (int i = 0; i < 3; ++i)
                {
                    if (u[i] == 0.0f)
                    {
                        seed[i] += rp;
                        break;
                    }
                }

                assert(eval(seed) == 0.0f);
                return { seed };
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

                Point min = mCentre - mRadius;
                Point max = mCentre + mRadius;

                return BBox(min, max);
            }

            float mRadius;
            atlas::math::Point mCentre;
        };
    }
}

#endif