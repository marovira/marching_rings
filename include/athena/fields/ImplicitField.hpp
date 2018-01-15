#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_IMPLICIT_FIELD_HPP

#pragma once

#include "Fields.hpp"
#include "Filters.hpp"
#include "athena/core/Math.hpp"
#include "athena/core/BBox.hpp"

namespace athena
{
    namespace fields
    {
        class ImplicitField
        {
        public:
            ImplicitField() :
                mFilter(wyvill)
            { }

            virtual ~ImplicitField() = default;

            virtual core::BBox getBBox() const = 0;

            void setFilter(FilterFn const& filter)
            {
                mFilter = filter;
            }

            float eval(core::Point const& p) const
            {
                return wyvill(sdf(p));
            }

            virtual core::Normal grad(core::Point const& p) const = 0;
            virtual core::Point getSeed() const = 0;

        protected:
            virtual float sdf(core::Point const& p) const = 0;

        private:
            FilterFn mFilter;

        };
    }
}


#endif