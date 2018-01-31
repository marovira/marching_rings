#include "athena/polygonizer/CrossSection.hpp"
#include "athena/polygonizer/Hash.hpp"
#include "athena/polygonizer/Tables.hpp"

#include <atlas/core/Float.hpp>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace athena
{
    namespace polygonizer
    {
        CrossSection::CrossSection(SlicingAxes const& axis, 
            atlas::math::Point const& min, atlas::math::Point const& max, 
            std::uint32_t gridSize, std::uint32_t svSize, 
            tree::BlobTree* tree) :
            mAxis(axis),
            mMin(min),
            mMax(max),
            mGridSize(gridSize),
            mSvSize(svSize),
            mTree(tree)
        {
            using atlas::math::Normal;

            assert(gridSize % svSize == 0);

            auto const& start = min;
            auto const& end = max;

            mGridDelta = (end - start) / static_cast<float>(mGridSize);
            mSvDelta = (end - start) / static_cast<float>(mSvSize);

            // Set our normal.
            switch (axis)
            {
            case SlicingAxes::XAxis:
                mNormal = Normal(start.x, 0, 0);
                mAxisId = glm::uvec2(1, 2);
                break;

            case SlicingAxes::YAxis:
                mNormal = Normal(0, start.y, 0);
                mAxisId = glm::uvec2(0, 2);
                break;

            case SlicingAxes::ZAxis:
                mNormal = Normal(0, 0, start.z);
                mAxisId = glm::uvec2(0, 1);
                break;
            }
        }

        void CrossSection::constructLattice()
        {
            using atlas::math::Point;
            using atlas::utils::BBox;

            std::uint32_t numSvs = mGridSize / mSvSize;

            // Can be done in parallel.
            for (std::uint32_t x = 0; x < numSvs; ++x)
            {
                for (std::uint32_t y = 0; y < numSvs; ++y)
                {
                    auto pt = createCellPoint(x, y, mSvDelta);

                    // Construct the cell that corresponds to the super-voxel.
                    BBox cell(pt, pt + mSvDelta);

                    SuperVoxel sv;
                    sv.fields = mTree->getOverlappingFields(cell);
                    sv.id = { x, y };

                    if (!sv.fields.empty())
                    {
                        auto idx = BsoidHash32::hash(x, y);
                        mSuperVoxels[idx] = sv;
                    }
                }
            }

            auto seedPoints = mTree->getSeeds(mNormal);
            std::vector<Voxel> seedVoxels;
            for (auto& pt : seedPoints)
            {
                auto v = (pt - mMin) / mGridDelta;
                PointId id;
                id.x = static_cast<std::uint32_t>(v[mAxisId.x]);
                id.y = static_cast<std::uint32_t>(v[mAxisId.y]);
                seedVoxels.emplace_back(id);
            }

            marchVoxelOnSurface(seedVoxels);
        }

        std::vector<Voxel> const& CrossSection::getVoxels() const
        {
            return mVoxels;
        }

        atlas::math::Point CrossSection::createCellPoint(std::uint32_t x,
            std::uint32_t y, atlas::math::Point const& delta)
        {
            atlas::math::Point pt;
            auto const& start = mMin;

            pt = start;
            // Figure out if there is a way to convert this to vector code.
            pt[mAxisId.x] = start[mAxisId.x] + x * delta[mAxisId.x];
            pt[mAxisId.y] = start[mAxisId.y] + x * delta[mAxisId.y];

            return pt;
        }

        atlas::math::Point CrossSection::createCellPoint(glm::u32vec2 const& p,
            atlas::math::Point const& delta)
        {
            return createCellPoint(p.x, p.y, delta);
        }

        void CrossSection::marchVoxelOnSurface(std::vector<Voxel> const& seeds)
        {
            using atlas::math::Point4;

            std::map<std::uint32_t, VoxelPoint> seenPoints;
            std::map<std::uint32_t, VoxelId> seenVoxels;
            std::queue<PointId> frontier;

            // First fill in the values of each of the seeds and insert them.
            {
                for (auto& seed : seeds)
                {
                    int i = 0;
                    for (auto& decal : VoxelDecals)
                    {
                        auto decalId = seed.id + decal;
                        auto hash = BsoidHash32::hash(decalId.x, decalId.y);
                        auto pt = createCellPoint(decalId, mGridDelta);

                        // TODO: Compute the field value for the point and then
                        // insert them into our list of seen voxels.
                    }

                    frontier.push(seed.id);
                }
            }

            auto generateVoxel = [this, &seenPoints](Voxel& v)
            {
                int d = 0;
                for (auto& decal : VoxelDecals)
                {
                    auto decalId = v.id + decal;
                    auto entry = seenPoints.find(
                        BsoidHash32::hash(decalId.x, decalId.y));
                    if (entry != seenPoints.end())
                    {
                        // We have seen this point already, so grab it from our list.
                        v.points[d] = (*entry).second;
                    }
                    else
                    {
                        // We haven't seen this point yet, so we need to add it
                        // to our list.
                        auto pt = createCellPoint(decalId, mGridDelta);
                        
                        // TODO: evaluate the point using the corresponding
                        // super-voxel.
                    }
                    ++d;
                }
            };

            auto getEdges = [this](Voxel const& v)
            {
                VoxelPoint start, end;
                int edgeId = 0;
                std::vector<int> edges;

                for (std::size_t i = 0; i < v.points.size(); ++i)
                {
                    start = v.points[i];
                    end = v.points[(i + 1) % v.points.size()];

                    // All that we care about is the change in sign. If there
                    // is a change, we know the surface crosses this edge.
                    if (glm::sign(start.value.w) != glm::sign(end.value.w))
                    {
                        edges.push_back(edgeId);
                    }
                    edgeId++;
                }

                return edges;
            };

            while (!frontier.empty())
            {
                // Grab a voxel from the queue.
                auto top = frontier.front();
                frontier.pop();

                // Check if we have seen this voxel before.
                if (seenVoxels.find(BsoidHash32::hash(top.x, top.y)) !=
                    seenVoxels.end())
                {
                    // We have, so do nothing.
                    continue;
                }
                else
                {
                    // We haven't seen it before, so add it to our list and
                    // proceed.
                    seenVoxels.insert(
                        std::pair<std::uint32_t, VoxelId>(
                            BsoidHash32::hash(top.x, top.y), top));
                }

                // Now fill its values.
                Voxel v(top);
                generateVoxel(v);

                // Check how many edges cross the surface.
                auto edges = getEdges(v);
                if (edges.empty())
                {
                    // There are no crossings, so continue;
                    continue;
                }

                // For each edge that crosses the surface, we need to add the 
                // corresponding voxel to our queue.
                for (auto& edge : edges)
                {
                    // Grab the decal for the corresponding neighbour.
                    auto decal = EdgeDecals.at(edge);

                    // Now get the corresponding voxel id.
                    auto neighbourDecal = v.id;
                    neighbourDecal.x += decal.x;
                    neighbourDecal.y = decal.y;

                    // Make sure that we don't run off the edge of the grid.
                    if (!Voxel(neighbourDecal).isValid())
                    {
                        continue;
                    }

                    frontier.push(neighbourDecal);
                }

                mVoxels.push_back(v);
            }
        }

        void CrossSection::validateVoxels() const
        {
            std::map<std::uint32_t, Voxel> seenMap;
            for (auto& voxel : mVoxels)
            {
                auto hash = BsoidHash32::hash(voxel.id.x, voxel.id.y);
                seenMap.insert(std::pair<std::uint32_t, Voxel>(hash, voxel));
            }

            assert(seenMap.size() == mVoxels.size());
        }
    }
}