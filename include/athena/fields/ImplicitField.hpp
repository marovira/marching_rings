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

            core::BBox getBBox() const
            {
                auto b = box();
                b.expand(1.0f);
                return b;
            }

            void setFilter(FilterFn const& filter)
            {
                mFilter = filter;
            }

            float eval(core::Point const& p) const
            {
                return mFilter(sdf(p));
            }

            virtual core::Normal grad(core::Point const& p) const = 0;
            virtual core::Point getSeed() const = 0;

        protected:
            virtual float sdf(core::Point const& p) const = 0;
            virtual core::BBox box() const = 0;

        private:
            FilterFn mFilter;

        };
    }
}


#endif