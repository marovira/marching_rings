#include "athena/blob/SuperVoxel.hpp"

namespace athena
{
    namespace blob
    {
        void SuperVoxel::setFields(
            std::vector<fields::ImplicitFieldPtr> const& fields)
        {
            mFields = fields;
        }

        void SuperVoxel::setVolume(core::BBox const& volume)
        {
            mVolume = volume;
        }

        core::BBox SuperVoxel::getVolume() const
        {
            return mVolume;
        }

        float SuperVoxel::eval(core::Point const& p) const
        {
            float value = 0.0f;
            for (auto& field : mFields)
            {
                value += field->eval(p);
            }

            return value;
        }

        core::Normal SuperVoxel::grad(core::Point const& p) const
        {
            core::Normal gradient;
            for (auto& field : mFields)
            {
                gradient += field->grad(p);
            }

            return gradient;
        }
    }
}