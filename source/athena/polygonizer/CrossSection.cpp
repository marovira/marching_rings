#include "athena/polygonizer/CrossSection.hpp"
#include "athena/polygonizer/Hash.hpp"

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
                auto v = pt / mGridDelta;
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