#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP

#pragma once

#include "Fields.hpp"
#include "athena/core/Math.hpp"
#include "athena/core/BBox.hpp"

namespace athena
{
    namespace fields
    {
        template <FilterFn filter>
        class ImplicitField
        {
        public:
            ImplicitField() = default;
            virtual ~ImplicitField() = default;

            virtual core::BBox getBBox() const = 0;

            float eval(core::Point const& p) const
            {
                return filter(sdf(p));
            }

            virtual core::Normal grad(core::Point const& p) const = 0;
            virtual core::Point getSeed() const;

        protected:
            virtual float sdf(core::Point const& p) const = 0;

        };
    }
}


#endif