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
            enum class BranchType : int
            {
                Cap = 0,
                Branch
            };

            struct BranchData
            {
                BranchData(std::size_t const& t, std::size_t const& b,
                    BranchType const& tp) :
                    top(t),
                    bottom(b),
                    type(tp)
                { }

                std::size_t top;
                std::size_t bottom;
                BranchType type;
            };

            BranchingManager();
            ~BranchingManager() = default;

            std::vector<BranchData> findBranches(
                std::vector<std::size_t> const& slices);
            void insertContours(std::vector<std::vector<FieldPoint>> const& slice);

            atlas::utils::Mesh linkContours();
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