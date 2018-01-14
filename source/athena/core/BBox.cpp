#include "mr/core/BBox.hpp"
#include "mr/core/Constants.hpp"
#include "mr/core/Float.hpp"

namespace mr
{
    namespace core
    {
        BBox::BBox() :
            pMin(mr::core::infinity()),
            pMax(mr::core::negInfinity())
        { }

        BBox::BBox(core::Point const& p) :
            pMin(p),
            pMax(p)
        { }

        BBox::BBox(core::Point const& p1, core::Point const& p2)
        {
            pMin = core::Point(
                glm::min(p1.x, p2.x),
                glm::min(p1.y, p2.y),
                glm::min(p1.z, p2.z)
                );

            pMax = core::Point(
                glm::max(p1.x, p2.x),
                glm::max(p1.y, p2.y),
                glm::max(p1.z, p2.z)
                );

        }

        bool BBox::overlaps(BBox const& b) const
        {
            bool x = (geq(pMax.x, b.pMin.x) && leq(pMin.x, b.pMax.x));
            bool y = (geq(pMax.y, b.pMin.y) && leq(pMin.y, b.pMax.y));
            bool z = (geq(pMax.z, b.pMin.z) && leq(pMin.z, b.pMax.z));

            return (x && y && z);
        }

        bool BBox::inside(core::Point const& p) const
        {
            return (
                geq(p.x, pMin.x) && leq(p.x, pMax.x) &&
                geq(p.y, pMin.y) && leq(p.y, pMax.y) &&
                geq(p.z, pMin.z) && leq(p.z, pMax.z));
        }

        void BBox::expand(float delta)
        {
            pMin -= delta;
            pMax += delta;
        }

        BBox join(BBox const& b1, BBox const& b2)
        {
            BBox ret = b1;
            ret.pMin.x = glm::min(b1.pMin.x, b2.pMin.x);
            ret.pMin.y = glm::min(b1.pMin.y, b2.pMin.y);
            ret.pMin.z = glm::min(b1.pMin.z, b2.pMin.z);

            ret.pMax.x = glm::max(b1.pMax.x, b2.pMax.x);
            ret.pMax.y = glm::max(b1.pMax.y, b2.pMax.y);
            ret.pMax.z = glm::max(b1.pMax.z, b2.pMax.z);

            return ret;
        }
    }
}