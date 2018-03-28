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

            atlas::utils::BBox getBBox() const override
            {
                return box();
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

            float eval(atlas::math::Point const& p) const override
            {
                return sdf(p);
            }

            atlas::math::Point grad(atlas::math::Point const& p) const override
            {
                return sdg(p);
            }

        protected:
            virtual ImplicitOperator* cloneEmpty() const = 0;

            std::vector<fields::ImplicitFieldPtr> mFields;
        };

    }
}

#endif