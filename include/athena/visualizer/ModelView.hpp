#ifndef ATHENA_INCLUDE_ATHENA_VISUALIZER_MODEL_VIEW_HPP
#define ATHENA_INCLUDE_ATHENA_VISUALIZER_MODEL_VIEW_HPP

#pragma once

#include "athena/polygonizer/Bsoid.hpp"
#include "athena/polygonizer/MarchingCubes.hpp"

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
            ModelView(polygonizer::Bsoid&& soid, polygonizer::MarchingCubes&& mc);
            ModelView(ModelView&& view) = default;
            ~ModelView() = default;

            std::string getModelName() const;

            void renderGeometry() override;
            void drawGui() override;

        private:
            void constructLattices();
            void constructContours();
            void constructMesh();
            void constructMCMesh();

            polygonizer::Bsoid mSoid;
            polygonizer::MarchingCubes mMC;

            atlas::gl::VertexArrayObject mLatticeVao;
            atlas::gl::Buffer mLatticeData;
            atlas::gl::Buffer mLatticeIndices;
            std::size_t mLatticeNumIndices;

            atlas::gl::VertexArrayObject mContourVao;
            atlas::gl::Buffer mContourData;
            atlas::gl::Buffer mContourIndices;
            std::size_t mContourNumIndices;
            std::size_t mContourNumVertices;

            atlas::gl::VertexArrayObject mMeshVao;
            atlas::gl::Buffer mMeshData;
            atlas::gl::Buffer mMeshIndices;
            std::size_t mMeshNumIndices;
            std::size_t mMeshNumVertices;

            atlas::gl::VertexArrayObject mMCVao;
            atlas::gl::Buffer mMCData;
            atlas::gl::Buffer mMCIndices;
            std::size_t mMCNumIndices;
            std::size_t mMCNumVertices;

            bool mShowLattices;
            bool mShowContours;
            bool mShowMesh;
            bool mShowMCMesh;
            int mRenderMode;
        };
    }
}

#endif