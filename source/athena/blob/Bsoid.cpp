#include "athena/blob/Bsoid.hpp"
#include "athena/blob/Hash.hpp"
#include "athena/blob/Voxel.hpp"
#include "athena/blob/Tables.hpp"
#include "athena/core/Log.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <queue>
#include <fstream>
#include <sstream>

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
            mMin = start;
            mMax = end;
            mGridSize = gridSize;
            mSvSize = svSize;

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

                for (auto& decal : VoxelDecals)
                {
                    auto decalId = v.id + decal;
                    if (!isValidId(decalId))
                    {
                        continue;
                    }

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
                        if (superVoxelList.find(svHash) == superVoxelList.end())
                        {
                            ERROR_LOG("Something went wrong!");
                            __debugbreak();
                        }
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
                    continue;
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
                    auto decals = EdgeDecals[edge];

                    // Now get the corresponding voxel id.
                    for (auto& decal : decals)
                    {
                        auto neighbourDecal = v.id;
                        neighbourDecal.x += decal.x;
                        neighbourDecal.y += decal.y;
                        neighbourDecal.z += decal.z;

                        if (!isValidId(neighbourDecal))
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
            struct CellModel
            {
                CellModel() = default;

                void constructModel(std::vector<Voxel> const& voxels)
                {
                    using core::Point;

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

                        idxs.push_back(start + 4);
                        idxs.push_back(start + 5);

                        idxs.push_back(start + 5);
                        idxs.push_back(start + 6);

                        idxs.push_back(start + 6);
                        idxs.push_back(start + 7);

                        idxs.push_back(start + 7);
                        idxs.push_back(start + 4);

                        idxs.push_back(start);
                        idxs.push_back(start + 4);

                        idxs.push_back(start + 1);
                        idxs.push_back(start + 5);

                        idxs.push_back(start + 2);
                        idxs.push_back(start + 6);

                        idxs.push_back(start + 3);
                        idxs.push_back(start + 7);

                        start = static_cast<std::uint32_t>(verts.size());
                    }

                    struct LinePoint
                    {
                        LinePoint(Point const& p) :
                            point(p)
                        { }

                        LinePoint(Point const& p, std::uint32_t i) :
                            point(p),
                            index(i)
                        { }

                        bool operator==(LinePoint const& rhs) const
                        {
                            return point == rhs.point;
                        }

                        bool operator!=(LinePoint const& rhs) const
                        {
                            return !(*this == rhs);
                        }

                        std::uint32_t index;
                        Point point;
                    };

                    struct PointHash
                    {
                        std::size_t operator()(LinePoint const& p) const
                        {
                            return std::hash<float>()(p.point.x + p.point.y +
                                p.point.z);
                        }
                    };

                    std::unordered_set<LinePoint, PointHash> uniqueVertices;
                    std::uint32_t current = 0;
                    for (auto& idx : idxs)
                    {
                        // Check if we have seen this vertex before.
                        auto pt = verts[idx];
                        if (uniqueVertices.find(pt) == uniqueVertices.end())
                        {
                            // We haven't, so increase the count and insert.
                            indices.push_back(current);
                            vertices.push_back(pt);

                            uniqueVertices.emplace(pt, current);
                            current++;
                        }
                        else
                        {
                            // We have seen this vertex, so grab it's index.
                            auto uniqueV = *uniqueVertices.find(pt);
                            indices.push_back(uniqueV.index);
                        }
                    }
                }

                std::vector<core::Point> vertices;
                std::vector<std::uint32_t> indices;
            };

            std::string filename = mName + ".obj";
            std::fstream file(filename, std::fstream::out);

            CellModel model;
            model.constructModel(mVoxels);

            file << "# number of vertices: " << model.vertices.size() << "\n";
            for (auto& vertex : model.vertices)
            {
                file << "v " << vertex.x << " " << vertex.y << " " << 
                    vertex.z << "\n";
            }

            file.close();
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

        bool Bsoid::isValidId(PointId const& id)
        {
            // First things first, check if the id isn't negative.
            if (!Voxel(id).isValid())
            {
                return false;
            }

            // Now check that it hasn't run off the edge of the grid on the
            // positive side.
            PointId maxId(mGridSize);
            return (
                id.x < maxId.x &&
                id.y < maxId.y &&
                id.z < maxId.z
                );
        }
    }
}