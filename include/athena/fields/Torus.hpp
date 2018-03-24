#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_TORUS_HPP

#pragma once

#include "ImplicitField.hpp"

#include <atlas/core/Float.hpp>

namespace athena
{
    namespace fields
    {
        class Torus : public ImplicitField
        {
        public:
            Torus() :
                mInner(2.0f),
                mOuter(1.0f),
                mCentre(0.0f)
            { }

            Torus(float inner, float outer, atlas::math::Point const& centre) :
                mInner(inner),
                mOuter(outer),
                mCentre(centre)
            { }

            ~Torus() = default;


            atlas::math::Normal grad(atlas::math::Point const& p) const override
            {
                using atlas::math::Normal;

                float sqrt = glm::length(p.xy() - mCentre.xy());
                float dx = 2.0f * (mInner - sqrt) * (p.x - mCentre.x) / sqrt;
                float dy = 2.0f * (mInner - sqrt) * (p.y - mCentre.y) / sqrt;
                float dz = 2.0f * (p.z - mCentre.z);

                return Normal(dx, dy, dz);
            }

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u, float offset) const override
            {
                using atlas::math::Point;
                using atlas::core::leq;
                using atlas::core::geq;

                // First project the centre onto the plane.
                auto v = mCentre - u;
                auto d = glm::proj(v, glm::normalize(u));
                auto proj = mCentre - d;

                float inner = mInner + offset;
                float outer = mOuter + offset;
                float l = glm::length(proj - mCentre);
                if (l > (inner + outer))
                {
                    return {};
                }

                // Check which axis we are slicing along.
                float rp = 0.0f;
                std::vector<Point> seeds;
                auto translatePoint = [u](atlas::math::Point& seed, float rp)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        if (u[i] == 0.0f)
                        {
                            seed[i] += rp;
                            break;
                        }
                    }
                };

                if (u.y != 0.0f || u.x != 0.0f)
                {
                    // Both x and y behave the same way.
                    // Check to see if we are between the two radii.
                    if (leq(inner, l))
                    {
                        // We haven't reached the hole of the torus yet, so 
                        // treat it like a sphere.
                        rp = glm::sqrt((outer * outer) - glm::length2(proj - mCentre));
                        Point seed = proj;
                        translatePoint(seed, rp);
                        seeds.push_back(seed);
                    }
                    else
                    {
                        // We are in the hole area, so we need to 
                        // return two points.
                        rp = glm::sqrt((inner * inner) - glm::length2(proj - mCentre));
                        Point seed = proj;
                        translatePoint(seed, rp);
                        seeds.push_back(seed);

                        rp *= -1.0f;
                        seed = proj;
                        translatePoint(seed, rp);
                        seeds.push_back(seed);
                    }
                }
                else
                {
                    // Slicing along the z axis.
                    rp = glm::sqrt((outer * outer) - glm::length2(proj - mCentre));
                    Point seed = proj;
                    translatePoint(seed, rp);
                    seeds.push_back(seed);

                    rp = glm::sqrt((inner * inner) - glm::length2(proj - mCentre));
                    seed = proj;
                    translatePoint(seed, rp);
                    seeds.push_back(seed);
                }

                return seeds;
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                using atlas::math::Point;
                using atlas::math::Point2;

                float root = glm::length(p.xy() - mCentre.xy());
                float z = glm::length(p.z - mCentre.z);
                float left = (mInner - root) * (mInner - root);
                return left + z - (mOuter * mOuter);
            }

            atlas::utils::BBox box() const override
            {
                using atlas::math::Point;

                Point p = {  mInner + mOuter,  mInner + mOuter, -mOuter };
                Point q = { -mInner - mOuter, -mInner - mOuter,  mOuter };
                return atlas::utils::BBox(p, q);
            }

            float mInner, mOuter;
            atlas::math::Point mCentre;
        };
    }
}


#endif