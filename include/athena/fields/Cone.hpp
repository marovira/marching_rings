#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_CONE_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_CONE_HPP

#pragma once

#include "ImplicitField.hpp"

namespace athena
{
    namespace fields
    {
        class Cone : public ImplicitField
        {
        public:
            Cone() :
                mRadius(1.0f),
                mHeight(1.0f)
            { }

            Cone(float radius, float height) :
                mRadius(radius),
                mHeight(height)
            { }


            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const override
            {
                // First project the centre onto the plane.
                auto centre = atlas::math::Point(mCentre.x, 0.0f, mCentre.y);
                auto v = centre - u;
                auto d = glm::proj(v, glm::normalize(u));
                auto proj = centre - d;

                float radius = mRadius;
                float l = glm::length(proj - centre);
                if (l > radius)
                {
                    return {};
                }

                float rp =
                    glm::sqrt((radius * radius) - glm::length2(proj - centre));
                auto seed = proj;
                for (int i = 0; i < 3; ++i)
                {
                    if (u[i] == 0.0f)
                    {
                        seed[i] += rp;
                        break;
                    }
                }

                return { seed };
            }

        private:
            float sdf(atlas::math::Point const& p) const override
            {
                float denom = glm::length2(p.xz());
                float c = mRadius / mHeight;

                return (denom / (c * c)) - (p.y * p.y);

            }

            atlas::math::Normal sdg(atlas::math::Point const& p) const override
            {
                auto g = p.xz() - mCentre;
                return { 2.0f * g.x, 0.0f, -2.0f * g.y };

            }

            atlas::utils::BBox box() const override
            {
                using atlas::utils::BBox;
                using atlas::math::Point;

                Point min = Point(mCentre.x, 0.0f, mCentre.y) - mRadius;
                Point max = Point(mCentre.x, mHeight, mCentre.y) + mRadius;

                return BBox(min, max);
            }

            float mRadius, mHeight;
            atlas::math::Point2 mCentre;
        };
    }
}

#endif