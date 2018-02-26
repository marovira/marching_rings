#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_IMPLICIT_OPERATOR_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_IMPLICIT_OPERATOR_HPP

#pragma once

#include "Operators.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>

namespace athena
{
    namespace operators
    {
        class ImplicitOperator : public fields::ImplicitField
        {
        public:
            ImplicitOperator() = default;
            virtual ~ImplicitOperator() = default;

            ImplicitOperatorPtr makeEmpty() const
            {
                return ImplicitOperatorPtr(cloneEmpty());
            }

            void insertField(fields::ImplicitFieldPtr const& field)
            {
                mFields.push_back(field);
            }

            void insertFields(
                std::vector<fields::ImplicitFieldPtr> const& fields)
            {
                for (auto& field : fields)
                {
                    insertField(field);
                }
            }

            virtual atlas::math::Normal grad(
                atlas::math::Point const& p) const = 0;

        protected:
            virtual float sdf(atlas::math::Point const& p) const = 0;
            virtual atlas::utils::BBox box() const = 0;
            virtual ImplicitOperator* cloneEmpty() const = 0;

            std::vector<fields::ImplicitFieldPtr> mFields;

        };

    }
}

#endif