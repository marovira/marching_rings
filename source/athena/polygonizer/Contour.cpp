#include "athena/polygonizer/Contour.hpp"

namespace athena
{
    namespace polygonizer
    {
        void Contour::makeContour(
            std::vector<std::vector<FieldPoint>> const& contours)
        {
            using atlas::math::Point;

            clearBuffers();

            std::uint32_t start = 0;
            std::uint32_t startIdx = 0;
            std::uint32_t numVerts = 0;
            std::uint32_t numIndices = 0;

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
                    numIndices += 2;
                }

                indexOffsets.emplace_back(startIdx, numIndices);
                vertexOffsets.emplace_back(start, numVerts);
                start += numVerts;
                startIdx += numIndices;
                numVerts = 0;
                numIndices = 0;
            }
        }

        void Contour::clearBuffers()
        {
            vertices.clear();
            indices.clear();
            indexOffsets.clear();
            vertexOffsets.clear();
        }
    }
}