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
                mC(2.0f),
                mA(1.0f),
                mCentre(0.0f)
            { }

            Torus(float inner, float outer, atlas::math::Point const& centre) :
                mC(inner),
                mA(outer),
                mCentre(centre)
            { }

            ~Torus() = default;

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const override
            {
                using atlas::math::Point;
                using atlas::core::leq;
                using atlas::core::geq;

                // First project the centre onto the plane.
                auto v = mCentre - u;
                auto d = glm::proj(v, glm::normalize(u));
                auto proj = mCentre - d;

                float c = mC;
                float a = mA;
                float R = c + a;
                float r = c - a;
                float l = glm::length(proj - mCentre);
                if (l > R)
                {
                    return {};
                }

                // Check which axis we are slicing along.
                std::vector<Point> seeds;
                Point seed1 = proj, seed2 = proj;
                if (u.y != 0.0f || u.x != 0.0f)
                {
                    float rp =
                        glm::sqrt((R * R) - (l * l));
                    int axis = (u.y != 0.0f) ? 0 : 1;
                    seed1[axis] += rp;
                    seed2[axis] -= rp;
                }
                if (u.z != 0.0f)
                {
                    float rp1 = glm::sqrt((r * r) - (l * l));
                    float rp2 = glm::sqrt((R * R) - (l * l));
                    seed1.x += rp1;
                    seed2.x += rp2;
                }

                seeds.push_back(seed1);
                seeds.push_back(seed2);

                return seeds;
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                using atlas::math::Point;
                using atlas::math::Point2;

                float root = glm::length(p.xy() - mCentre.xy());
                float z2 = (p.z - mCentre.z) * (p.z - mCentre.z);
                float left = (mC - root) * (mC - root);
                return left + z2 - (mA * mA);
            }

            atlas::math::Normal sdg(atlas::math::Point const& p) const override
            {
                using atlas::math::Normal;

                float sqrt = glm::length(p.xy() - mCentre.xy());
                float dx = -2.0f * (mC - sqrt) * (p.x - mCentre.x) / sqrt;
                float dy = -2.0f * (mC - sqrt) * (p.y - mCentre.y) / sqrt;
                float dz = 2.0f * (p.z - mCentre.z);

                return Normal(dx, dy, dz);
            }

            atlas::utils::BBox box() const override
            {
                using atlas::math::Point;

                Point p = {  mC + mA,  mC + mA, -mA };
                Point q = { -mC - mA, -mC - mA,  mA };
                return atlas::utils::BBox(p, q);
            }

            float mC, mA;
            atlas::math::Point mCentre;
        };
    }
}


#endif