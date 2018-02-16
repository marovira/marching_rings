#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_IMPLICIT_OPERATOR_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_IMPLICIT_OPERATOR_HPP

#pragma once

#include "Operators.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>
#include <array>

namespace athena
{
    namespace operators
    {
        template <std::size_t Arity>
        class ImplicitOperator : public fields::ImplicitField
        {
        public:
            ImplicitOperator() = default;
            virtual ~ImplicitOperator() = default;

            void insertField(fields::ImplicitFieldPtr const& field)
            {
                if (mIndex < mFields.size())
                {
                    mFields[mIndex] = field;
                    mIndex++;
                }
            }

            virtual atlas::utils::BBox getBBox() const = 0;
            virtual atlas::math::Normal grad(
                atlas::math::Point const& p) const = 0;
            virtual std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const = 0;

        protected:
            virtual float sdf(atlas::math::Point const& p) const = 0;

            std::array<fields::ImplicitFieldPtr, Arity> mFields;
            std::size_t mIndex;
        };

        template <>
        class ImplicitOperator<0> : public fields::ImplicitField
        {
            ImplicitOperator() = default;
            virtual ~ImplicitOperator() = default;

            void insertField(fields::ImplicitFieldPtr const& field)
            {
                mFields.push_back(field);
            }

            virtual atlas::utils::BBox getBBox() const = 0;
            virtual atlas::math::Normal grad(
                atlas::math::Point const& p) const = 0;

        protected:
            virtual float sdf(atlas::math::Point const& p) const = 0;
            std::vector<fields::ImplicitFieldPtr> mFields;
        };

    }
}

#endif