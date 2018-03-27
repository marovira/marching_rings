#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_MARCHING_CUBES_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_MARCHING_CUBES_HPP

#pragma once

#include "Polygonizer.hpp"
#include "athena/tree/BlobTree.hpp"

#include <atlas/utils/Mesh.hpp>

#include <sstream>
#include <string>
#include <vector>
#include <cinttypes>

namespace athena
{
    namespace polygonizer
    {
        class MarchingCubes
        {
        public:
            MarchingCubes();
            MarchingCubes(tree::BlobTree const& model, std::string const& name,
                float isoValue = 0.5f);
            MarchingCubes(MarchingCubes&& mc);

            ~MarchingCubes() = default;

            void setModel(tree::BlobTree const& tree);
            void setIsoValue(float isoValue);
            void setResolution(glm::u32vec3 const& res);

            void polygonize();

            atlas::utils::Mesh& getMesh();

            void setName(std::string const& name);
            std::string getName() const;

            std::string getLog() const;
            void clearLog();

            void saveMesh();

        private:
            struct VoxelPoint
            {
                VoxelPoint() = default;
                
                atlas::math::Vector4 data;
            };

            void constructGrid();
            void createTriangles();

            glm::u32vec3 mResolution;
            atlas::utils::Mesh mMesh;
            std::vector<std::vector<std::vector<VoxelPoint>>> mGrid;
            std::vector<atlas::math::Point> mVertices;
            std::vector<atlas::math::Normal> mNormals;
            std::vector<std::uint32_t> mIndices;
            tree::TreePointer mTree;
            float mMagic;

            std::stringstream mLog;
            std::string mName;

        };
    }
}

#endif