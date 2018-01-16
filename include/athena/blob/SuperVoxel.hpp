#ifndef ATHENA_INCLUDE_ATHENA_BLOB_SUPER_VOXEL_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_SUPER_VOXEL_HPP

#pragma once

#include "Blob.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>

namespace athena
{
    namespace blob
    {
        class SuperVoxel
        {
        public:
            SuperVoxel() = default;
            ~SuperVoxel() = default;

            void setFields(std::vector<fields::ImplicitFieldPtr> const& fields);
            void setVolume(core::BBox const& volume);
            core::BBox getVolume() const;

            float eval(core::Point const& p) const;
            core::Normal grad(core::Point const& p) const;

        private:
            std::vector<fields::ImplicitFieldPtr> mFields;
            core::BBox mVolume;
        };
    }
}

#endif