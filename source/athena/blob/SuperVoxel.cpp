#include "athena/blob/SuperVoxel.hpp"

namespace athena
{
    namespace blob
    {
        SuperVoxel::SuperVoxel(glm::u32vec3 const& i,
            std::vector<fields::ImplicitFieldPtr> const& f, 
            core::BBox const& v) :
            id(i),
            fields(f),
            volume(v)
        { }

        float SuperVoxel::eval(core::Point const& p) const
        {
            float value = 0.0f;
            for (auto& field : fields)
            {
                value += field->eval(p);
            }

            return value;
        }

        core::Normal SuperVoxel::grad(core::Point const& p) const
        {
            core::Normal gradient;
            for (auto& field : fields)
            {
                gradient += field->grad(p);
            }

            return gradient;
        }
    }
}