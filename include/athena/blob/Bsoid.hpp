#ifndef ATHENA_INCLUDE_ATHENA_BLOB_BSOID_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_BSOID_HPP

#pragma once

#include "Blob.hpp"
#include "SuperVoxel.hpp"
#include "athena/tree/BlobTree.hpp"

#include <string>

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

            void polygonize();

            void saveCubicLattice() const;

        private:
            tree::BlobTree mModel;
            std::string mName;
        };
    }
}

#endif