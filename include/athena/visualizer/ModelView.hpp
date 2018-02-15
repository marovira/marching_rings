#ifndef ATHENA_INCLUDE_ATHENA_VISUALIZER_MODEL_VIEW_HPP
#define ATHENA_INCLUDE_ATHENA_VISUALIZER_MODEL_VIEW_HPP

#pragma once

#include "athena/fields/ImplicitField.hpp"
#include "athena/polygonizer/Bsoid.hpp"

#include <atlas/utils/Geometry.hpp>
#include <atlas/utils/Mesh.hpp>
#include <atlas/gl/Buffer.hpp>
#include <atlas/gl/VertexArrayObject.hpp>

namespace athena
{
    namespace visualizer
    {
        class ModelView : public atlas::utils::Geometry
        {
        public:
            ModelView(polygonizer::Bsoid&& soid);
            ModelView(ModelView&& view) = default;
            ~ModelView() = default;

            std::string getModelName() const;

            void renderGeometry() override;
            void drawGui() override;

        private:
            void constructLattices();
            void constructContours();
            void constructMesh();

            polygonizer::Bsoid mSoid;

            atlas::gl::VertexArrayObject mLatticeVao;
            atlas::gl::Buffer mLatticeData;
            atlas::gl::Buffer mLatticeIndices;
            std::size_t mLatticeNumIndices;

            atlas::gl::VertexArrayObject mContourVao;
            atlas::gl::Buffer mContourData;
            atlas::gl::Buffer mContourIndices;
            std::size_t mContourNumIndices;
            std::size_t mContourNumVertices;

            bool mShowLattices;
            bool mShowContours;
            int mRenderMode;
        };
    }
}

#endif