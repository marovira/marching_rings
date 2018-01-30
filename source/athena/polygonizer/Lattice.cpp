#include "athena/polygonizer/Lattice.hpp"

#include <functional>
#include <unordered_set>

namespace athena
{
    namespace polygonizer
    {
        void Lattice::makeLattice(std::vector<Voxel> const& voxels)
        {
            using atlas::math::Point;

            std::vector<Point> verts;
            std::vector<std::uint32_t> idxs;
            std::uint32_t start = 0;
            for (auto& cell : voxels)
            {
                for (auto& pt : cell.points)
                {
                    verts.push_back(pt.value.xyz());
                }

                idxs.push_back(start);
                idxs.push_back(start + 1);

                idxs.push_back(start + 1);
                idxs.push_back(start + 2);

                idxs.push_back(start + 2);
                idxs.push_back(start + 3);

                idxs.push_back(start + 3);
                idxs.push_back(start);

                start = static_cast<std::uint32_t>(verts.size());
            }

            struct LatticePoint
            {
                LatticePoint(Point const& p) :
                    point(p)
                { }

                LatticePoint(Point const& p, std::uint32_t i) :
                    point(p),
                    index(i)
                { }

                bool operator==(LatticePoint const& rhs) const
                {
                    return point == rhs.point;
                }

                bool operator!=(LatticePoint const& rhs) const
                {
                    return !(*this == rhs);
                }

                Point point;
                std::uint32_t index;
            };

            struct LatticeHash
            {
                std::size_t operator()(LatticePoint const& p) const
                {
                    return std::hash<float>()(p.point.x + p.point.y + 
                        p.point.z);
                }
            };

            std::unordered_set<LatticePoint, LatticeHash> uniqueVertices;
            std::uint32_t current = 0;
            for (auto& idx : idxs)
            {
                auto pt = verts[idx];
                if (uniqueVertices.find(pt) == uniqueVertices.end())
                {
                    indices.push_back(current);
                    vertices.push_back(pt);

                    uniqueVertices.emplace(pt, current);
                    current++;
                }
                else
                {
                    auto uniqueV = *uniqueVertices.find(pt);
                    indices.push_back(uniqueV.index);
                }
            }
        }
    }
}