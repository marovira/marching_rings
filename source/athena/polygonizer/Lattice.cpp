#include "athena/polygonizer/Lattice.hpp"

#include <functional>
#include <unordered_set>

namespace athena
{
    namespace polygonizer
    {
        void Lattice::makeLattice(std::vector<std::vector<Voxel>> const& voxels)
        {
            using atlas::math::Point;

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

            for (auto& list : voxels)
            {
                if (list.empty())
                {
                    continue;
                }

                std::vector<Point> verts;
                std::vector<std::uint32_t> idxs;
                std::uint32_t idxStart = 0;
                for (auto& cell : list)
                {
                    for (auto& pt : cell.points)
                    {
                        verts.push_back(pt.value.xyz());
                    }

                    idxs.push_back(idxStart);
                    idxs.push_back(idxStart + 1);

                    idxs.push_back(idxStart + 1);
                    idxs.push_back(idxStart + 2);

                    idxs.push_back(idxStart + 2);
                    idxs.push_back(idxStart + 3);

                    idxs.push_back(idxStart + 3);
                    idxs.push_back(idxStart);
                    
                    idxStart = static_cast<std::uint32_t>(verts.size());
                }
                
                std::unordered_set<LatticePoint, LatticeHash> uniqueVertices;
                std::uint32_t current = vertices.size();
                std::uint32_t start = indices.size();
                std::uint32_t size = 0;
                for (auto& idx : idxs)
                {
                    auto pt = verts[idx];
                    if (uniqueVertices.find(pt) == uniqueVertices.end())
                    {
                        indices.push_back(current);
                        vertices.push_back(pt);

                        uniqueVertices.emplace(pt, current);
                        size++;
                        current++;
                    }
                    else
                    {
                        auto uniqueV = *uniqueVertices.find(pt);
                        indices.push_back(uniqueV.index);
                        size++;
                    }
                }

                offsets.emplace_back(start, size);
            }
        }
    }
}