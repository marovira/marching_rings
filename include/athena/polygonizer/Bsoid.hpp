#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_BSOID_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_BSOID_HPP

#pragma once

#include "Polygonizer.hpp"
#include "CrossSection.hpp"
#include "Lattice.hpp"
#include "Contour.hpp"
#include "athena/tree/BlobTree.hpp"

#include <atlas/utils/Mesh.hpp>

#include <sstream>
#include <string>
#include <cinttypes>
#include <vector>

namespace athena
{
    namespace polygonizer
    {
        class Bsoid
        {
        public:
            Bsoid();
            Bsoid(tree::BlobTree const& model, std::string const& name);
            Bsoid(Bsoid&& b);

            ~Bsoid() = default;

            void setModel(tree::BlobTree const& tree);

            void setCrossSectionDelta(float delta, SlicingAxes const& axis);
            void setNumCrossSections(std::size_t num, SlicingAxes const& axis);
            std::size_t numCrossSections() const;

            void makeCrossSections(SlicingAxes const& axis,
                std::uint32_t gridSize, std::uint32_t svSize);

            void constructLattices();
            void constructContours();
            void constructMesh();

            Lattice const& getLattice() const;
            Contour const& getContour() const;
            atlas::utils::Mesh& getMesh();

            void setName(std::string const& name);
            std::string getName() const;

            std::string getLog() const;
            void clearLog();

            void saveMesh();

        private:
            void connectContours();

            Lattice mLattice;
            Contour mContour;
            tree::TreePointer mTree;
            float mCrossSectionDelta;

            std::vector<CrossSectionPointer> mCrossSections;
            atlas::utils::Mesh mMesh;

            std::stringstream mLog;
            std::string mName;
        };
    }
}

#endif