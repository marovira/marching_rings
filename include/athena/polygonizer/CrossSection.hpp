#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_CROSS_SECTION_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_CROSS_SECTION_HPP

#pragma once

#include "Polygonizer.hpp"
#include "Voxel.hpp"
#include "SuperVoxel.hpp"
#include "LineSegment.hpp"
#include "athena/fields/ImplicitField.hpp"
#include "athena/tree/BlobTree.hpp"

#include <atlas/math/Math.hpp>
#include <atlas/core/Macros.hpp>

#include <cstdint>
#include <unordered_map>
#include <map>

namespace athena
{
    namespace polygonizer
    {
        class CrossSection
        {
        public:
            CrossSection(SlicingAxes const& axis, atlas::math::Point const& min,
                atlas::math::Point const& max, std::uint32_t gridSize,
                std::uint32_t svSize, float isoValue, tree::BlobTree* tree);
            ~CrossSection() = default;

            void constructLattice();
            void constructContour();
            void resizeContours(std::size_t size);

            std::vector<Voxel> const& getVoxels() const;
            std::vector<std::vector<FieldPoint>> const& getContour() const;
            std::size_t getLargestContourSize() const;

        private:
            atlas::math::Point createCellPoint(std::uint32_t x, std::uint32_t y,
                atlas::math::Point const& delta);
            atlas::math::Point createCellPoint(glm::u32vec2 const& p,
                atlas::math::Point const& delta);
            void fillVoxel(Voxel& v);

            FieldPoint findVoxelPoint(PointId const& id);
            Voxel findSurfaceVoxel(Voxel const& start);
            void marchVoxelOnSurface(std::vector<Voxel> const& seeds);

            std::vector<LineSegment> generateLineSegments(
                std::vector<Voxel> const& voxels);
            void convertToContour(std::vector<LineSegment> const& segments);
            void subdivideContour(int idx, std::size_t size);

            bool validVoxel(Voxel const& v) const;

            void validateVoxels() const;
            void validateContour() const;

            atlas::math::Point mGridDelta, mSvDelta, mMin, mMax;
            atlas::math::Normal mNormal;
            atlas::math::Normal mUnitNormal;
            std::uint32_t mGridSize, mSvSize;
            glm::uvec2 mAxisId;
            float mMagic;

            tree::BlobTree* mTree;
            SlicingAxes mAxis;

            std::vector<Voxel> mVoxels;
            std::vector<std::vector<FieldPoint>> mContours;

            std::unordered_map<std::uint32_t, SuperVoxel> mSuperVoxels;

            std::map<std::uint32_t, FieldPoint> mSeenVoxelPoints;
            std::map<std::uint32_t, VoxelId> mSeenVoxels;

            std::size_t mLargestContourSize;
        };
    }
}

#endif