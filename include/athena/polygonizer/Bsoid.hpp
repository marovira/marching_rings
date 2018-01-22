#ifndef ATHENA_INCLUDE_ATHENA_BLOB_BSOID_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_BSOID_HPP

#pragma once

#include "Blob.hpp"
#include "SuperVoxel.hpp"
#include "Voxel.hpp"
#include "athena/tree/BlobTree.hpp"

#include <string>
#include <cinttypes>

namespace athena
{
    namespace blob
    {
        class Bsoid
        {
        public:
            Bsoid();
            Bsoid(tree::BlobTree const& model, std::string const& name);
            ~Bsoid() = default;

            void setModel(tree::BlobTree const& model);
            void setName(std::string const& name);

            void polygonize(std::size_t svSize, std::size_t gridSize);

            void saveCubicLattice() const;

        private:
            template <typename T>
            core::Point createCellPoint(T x, T y, T z, core::Point const& delta)
            {
                core::Point pt;
                auto const& start = mMin;

                pt = start + core::Point(x, y, z) * delta;
                return pt;
            }

            core::Point createCellPoint(glm::u32vec3 const& p,
                core::Point const& delta);
            core::Point createCellPoint(glm::u64vec3 const& p,
                core::Point const& delta);

            bool isValidId(PointId const& id);

            core::Point  mMin, mMax;
            tree::BlobTree mModel;
            std::string mName;
            std::size_t mGridSize, mSvSize;

            core::Vector mGridDelta, mSvDelta;
            std::vector<Voxel> mVoxels;
        };
    }
}

#endif