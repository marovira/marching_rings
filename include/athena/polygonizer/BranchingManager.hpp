#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_BRANCHING_MANAGER_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_BRANCHING_MANAGER_HPP

#pragma once

#include "Polygonizer.hpp"
#include "Voxel.hpp"

#include <atlas/utils/Mesh.hpp>

namespace athena
{
    namespace polygonizer
    {
        class BranchingManager
        {
        public:
            BranchingManager();
            ~BranchingManager() = default;

            void insertContours(std::vector<std::vector<FieldPoint>> const& slice);
            atlas::utils::Mesh connectContours();

        private:
            using Slice = std::vector<std::vector<std::uint32_t>>;

            void singleBranch(Slice const& top, Slice const& bottom);
            void manyToManyBranch(Slice const& top, Slice const& bottom);
            void multiBranch(Slice const& top, Slice const& bottom);

            std::vector<Slice> mSlices;
            atlas::utils::Mesh mMesh;

        };
    }
}

#endif