#ifndef MR_INCLUDE_MR_CORE_BBOX_HPP
#define MR_INCLUDE_MR_CORE_BBOX_HPP

#pragma once

#include "Math.hpp"

namespace mr
{
    namespace core
    {
        class BBox
        {
        public:
            BBox();
            BBox(Point const& p);
            BBox(Point const& p1, core::Point const& p2);
            ~BBox() = default;

            bool overlaps(BBox const& b) const;

            bool inside(Point const& p) const;

            void expand(float delta);

            friend BBox join(BBox const& b1, BBox const& b2);

            Point pMin, pMax;
        };
    }
}

#endif