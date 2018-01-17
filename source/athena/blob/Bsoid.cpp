#include "athena/blob/Bsoid.hpp"
#include "athena/blob/Hash.hpp"
#include "athena/blob/Voxel.hpp"

#include <cassert>
#include <unordered_map>

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

            // Now that we have the super-voxels, let's grab all of the seeds
            // from our model.
            auto seeds = mModel.getSeeds();

            // For each seed, let's find the voxel that they belong to.
            for (auto& seed : seeds)
            {
                glm::u64vec3 id = glm::floor(seed / static_cast<float>(gridSize));
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