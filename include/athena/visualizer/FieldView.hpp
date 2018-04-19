#ifndef ATHENA_INCLUDE_ATHENA_VISUALIZER_FIELD_VIEW_HPP
#define ATHENA_INCLUDE_ATHENA_VISUALIZER_FIELD_VIEW_HPP

#pragma once

#include "athena/tree/BlobTree.hpp"
#include "athena/polygonizer/Polygonizer.hpp"

#include <atlas/utils/Geometry.hpp>
#include <atlas/utils/Mesh.hpp>
#include <atlas/gl/Buffer.hpp>
#include <atlas/gl/VertexArrayObject.hpp>

namespace athena
{
    namespace visualizer
    {
        class FieldView : public atlas::utils::Geometry
        {
        public:
            FieldView(tree::BlobTree* tree);
            FieldView(FieldView&& view) = default;
            ~FieldView() = default;

            void constructSlices(std::size_t numSlices, float sliceDelta,
                polygonizer::SlicingAxes const& axis);

            void renderGeometry() override;
            void drawGui() override;

        private:
            void initShaders();

            atlas::gl::VertexArrayObject mVao;
            atlas::gl::Buffer mSliceData;
            atlas::gl::Buffer mSliceIndices;
            std::size_t mSliceNumIndices;
            std::size_t mNumVertices;

            using IndexOffset = std::pair<std::size_t, std::size_t>;
            tree::BlobTree* mTree;
            std::size_t mNumSlices;
            std::vector<IndexOffset> mOffsets;
            int mSelectedSlice;
            int mRenderMode;
            bool mShowField;
        };
    }
}

#endif