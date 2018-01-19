#include "athena/blob/Bsoid.hpp"
#include "athena/blob/Hash.hpp"
#include "athena/blob/Voxel.hpp"

#include <cassert>
#include <unordered_map>
#include <map>
#include <queue>

namespace athena
{
    namespace blob
    {
        Bsoid::Bsoid() :
            mName("model")
        { }

        Bsoid::Bsoid(tree::BlobTree const& model, std::string const& name) :
            mModel(model),
            mName(name)
        { }

        void Bsoid::setModel(tree::BlobTree const& model)
        {
            mModel = model;
        }

        void Bsoid::setName(std::string const& name)
        {
            mName = name;
        }

        void Bsoid::polygonize(std::size_t svSize, std::size_t gridSize)
        {
            using core::Point;
            using core::BBox;

            // Ensure that the grid size is a multiple of the super-voxel size.
            assert(gridSize % svSize == 0);

            // Next compute the deltas for our super-voxels and grid.
            auto const& start = mModel.getTreeBox().pMin;
            auto const& end = mModel.getTreeBox().pMax;

            mGridDelta = (end - start) / static_cast<float>(gridSize);
            mSvDelta = (end - start) / static_cast<float>(svSize);

            std::unordered_map<std::uint32_t, SuperVoxel> superVoxelList;

            // First, let's generate the super-voxels.
            {
                std::uint32_t numSvs = gridSize / svSize;
                for (std::uint32_t x = 0; x < numSvs; ++x)
                {
                    for (std::uint32_t y = 0; y < numSvs; ++y)
                    {
                        for (std::uint32_t z = 0; z < numSvs; ++z)
                        {
                            auto pt = createCellPoint(
                                glm::u32vec3(x, y, z), mSvDelta);

                            BBox cell(pt, pt + mSvDelta);

                            SuperVoxel sv;
                            sv.fields = mModel.getOverlappingFields(cell);
                            sv.id = { x, y, z };
                            if (!sv.fields.empty())
                            {
                                auto idx = BsoidHash32::hash(x, y, z);
                                superVoxelList[idx] = sv;
                            }
                        }
                    }
                }
            }

            std::map<std::uint64_t, VoxelPoint> seenPoints;
            std::map<std::uint64_t, VoxelId> seenVoxels;

            auto generateVoxel = 
                [this, &seenPoints, svSize, gridSize, superVoxelList](Voxel& v)
            {
                int d = 0;
                std::vector<PointId> decals = 
                {
                    {0, 0, 0},
                    {1, 0, 0},
                    {1, 0, 1},
                    {0, 0, 1},
                    {0, 1, 0},
                    {1, 1, 0},
                    {1, 1, 1},
                    {0, 1, 1}
                };

                for (auto& decal : decals)
                {
                    auto decalId = v.id + decal;
                    auto entry = seenPoints.find(
                        BsoidHash64::hash(decalId.x, decalId.y, decalId.z));
                    if (entry != seenPoints.end())
                    {
                        v.points[d] = (*entry).second;
                    }
                    else
                    {
                        auto pt = createCellPoint(decalId, mGridDelta);
                        auto svId = decalId / svSize;
                        auto svHash = BsoidHash32::hash(svId.x, svId.y, svId.z);
                        auto sv = (*superVoxelList.find(svHash)).second;
                        VoxelPoint vp(pt, sv.eval(pt), sv.grad(pt));
                        auto hash = BsoidHash64::hash(decalId.x, decalId.y, decalId.z);
                        seenPoints.insert(
                            std::pair<std::uint64_t, VoxelPoint>(hash, vp));
                        v.points[d] = vp;
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
                    float val1 = start.value.w - 0.5f;
                    float val2 = end.value.w - 0.0f;
                    if (glm::sign(val1) != glm::sign(val2))
                    {
                        edges.push_back(edgeId);
                    }

                    edgeId++;
                }

                return edges;
            };

            // Now that we have the super-voxels, let's grab all of the seeds
            // from our model.
            auto seeds = mModel.getSeeds();

            std::queue<PointId> frontier;
            for (auto& seed : seeds)
            {
                PointId id = glm::floor(seed / static_cast<float>(gridSize));
                Voxel v;
                v.id = id;
                generateVoxel(v);
                frontier.push(id);
            }

            //std::map<int, glm::ivec3> edgeDecals;
            std::vector<std::vector<glm::ivec3>> edgeDecals;

            while (!frontier.empty())
            {
                // Grab a voxel from the queue.
                auto top = frontier.front();
                frontier.pop();

                // Check if we have seen this voxel before.
                if (seenVoxels.find(BsoidHash64::hash(top.x, top.y, top.z)) !=
                    seenVoxels.end())
                {
                    // We have, so do nothing.
                }
                else
                {
                    // We haven't seen it before, so add it to our list and
                    // proceed.
                    seenVoxels.insert(std::pair<std::uint64_t, VoxelId>(
                        BsoidHash64::hash(top.x, top.y, top.z), top));
                }

                // Now fill its values.
                Voxel v(top);
                generateVoxel(v);

                // Now check how many edges cross the surface.
                auto edges = getEdges(v);
                if (edges.empty())
                {
                    continue;
                }

                for (auto& edge : edges)
                {
                    // Grab the decal for the corresponding neighbour.
                    auto decals = edgeDecals[edge];

                    // Now get the corresponding voxel id.
                    for (auto& decal : decals)
                    {
                        auto neighbourDecal = v.id;
                        v.id.x += decal.x;
                        v.id.y += decal.y;
                        v.id.z += decal.z;

                        if (!Voxel(neighbourDecal).isValid())
                        {
                            continue;
                        }

                        frontier.push(neighbourDecal);
                    }
                }

                mVoxels.push_back(v);
            }
        }

        void Bsoid::saveCubicLattice() const
        {

        }

        core::Point Bsoid::createCellPoint(glm::u32vec3 const& p,
            core::Point const& delta)
        {
            return createCellPoint(p.x, p.y, p.z, delta);
        }

        core::Point Bsoid::createCellPoint(glm::u64vec3 const& p,
            core::Point const& delta)
        {
            return createCellPoint(p.x, p.y, p.z, delta);
        }
    }
}