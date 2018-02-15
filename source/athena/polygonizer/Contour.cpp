#include "athena/polygonizer/Contour.hpp"

namespace athena
{
    namespace polygonizer
    {
        void Contour::makeContour(
            std::vector<std::vector<FieldPoint>> const& contours)
        {
            using atlas::math::Point;

            std::uint32_t start = 0;
            std::uint32_t numVerts = 0;
            vertices.clear();
            indices.clear();

            for (auto& contour : contours)
            {
                for (auto& pt : contour)
                {
                    vertices.push_back(pt.value.xyz());
                    ++numVerts;
                }

                for (std::uint32_t i = 0; i < contour.size(); ++i)
                {
                    indices.push_back(start + (i + 0));
                    indices.push_back(start + ((i + 1) % contour.size()));
                }
                start += numVerts;
                numVerts = 0;
            }
        }
    }
}