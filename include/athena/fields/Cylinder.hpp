#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_CYLINDER_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_CYLINDER_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        class Cylinder : public ImplicitField
        {
        public:
            Cylinder() :
                mRadius(1.0f),
                mHeight(3.0f),
                mCentre(0.0f)
            { }

            Cylinder(float radius, float height, atlas::math::Point2 const& centre) :
                mRadius(radius),
                mHeight(height),
                mCentre(centre)
            { }

            atlas::math::Normal grad(atlas::math::Point const& p) const override
            {
                auto g = p.xz() - mCentre;
                return { 2.0f * g.x, 0.0f, 2.0f * g.y };
            }

            std::vector<atlas::math::Point> getSeeds(atlas::math::Normal const& u,
                float offset) const override
            {
                using atlas::math::Point;

                // First project the centre onto the plane.
                Point centre = Point(mCentre.x, 0.0f, mCentre.y);
                auto v = centre - u;
                auto d = glm::proj(v, glm::normalize(u));
                auto proj = centre - d;

                float radius = mRadius + offset;
                float l = glm::length(proj - centre);
                if (l > radius)
                {
                    return {};
                }

                // Now we need to see if we have to return 1 or 2 points depending
                // on the axis.
                auto seed = proj;
                if (u.y != 0.0f)
                {
                    // We only have to return 1 point.
                    seed.x += radius;
                    seed.z += radius;
                    return { seed };
                }

                // We need to return 2 seeds.
                seed.x += radius;
                seed.z += radius;

                auto seed2 = proj;
                seed2.x -= radius;
                seed2.z -= radius;

                return { seed, seed2 };
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                return glm::length(p.xz() - mCentre) - mRadius;
            }

            atlas::utils::BBox box() const override
            {
                using atlas::utils::BBox;
                using atlas::math::Point;

                Point min = Point(mCentre.x, -0.5f * mHeight, mCentre.y) - mRadius;
                Point max = Point(mCentre.x, 0.5f * mHeight, mCentre.y) + mRadius;

                return BBox(min, max);
            }

            float mRadius, mHeight;
            atlas::math::Point2 mCentre;
        };
    }
}

#endif
