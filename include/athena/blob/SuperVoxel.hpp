#ifndef ATHENA_INCLUDE_ATHENA_BLOB_SUPER_VOXEL_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_SUPER_VOXEL_HPP

#pragma once

#include "Blob.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>
#include <cinttypes>

namespace athena
{
    namespace blob
    {
        struct SuperVoxel
        {
            SuperVoxel() = default;
            SuperVoxel(glm::u32vec3 const& i, 
                std::vector<fields::ImplicitFieldPtr> const& f, 
                core::BBox const& v);
            ~SuperVoxel() = default;

            float eval(core::Point const& p) const;
            core::Normal grad(core::Point const& p) const;

            glm::u32vec3 id;
            std::vector<fields::ImplicitFieldPtr> fields;
            core::BBox volume;

        };
    }
}

#endif