#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_INTERSECTION_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_INTERSECTION_HPP

#pragma once

#include "Operators.hpp"
#include "ImplicitOperator.hpp"

#include <atlas/core/Constants.hpp>

namespace athena
{
    namespace operators
    {
        template <std::size_t Arity>
        class Intersection : public ImplicitOperator<Arity>
        {
            Intersection() = default;
            ~Intersection() = default;

            atlas::utils::BBox getBBox() const
            {
                atlas::utils::BBox box;
                for (auto& f : mFields)
                {
                    box = atlas::utils::join(box, f->getBBox());
                }

                return box;
            }

            atlas::math::Normal grad(atlas::math::Point const& p) const
            {
                atlas::math::Normal gradient(atlas::core::infinity());
                for (auto& f : mFields)
                {
                    gradient = glm::min(gradient, f->grad(p));
                }

                return gradient;
            }

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const
            {
                // TODO: Let's see if we can make something a bit more clever
                // here and detect when we are outside of the region of 
                // intersection.
                std::vector result;
                for (auto& f : mFields)
                {
                    result.push_back(f->getSeeds(u));
                }

                return result;
            }

        private:
            float sdf(atlas::math::Point const& p) const
            {
                float field = atlas::core::infinity();
                for (auto& f : mFields)
                {
                    field = glm::min(field, f->eval(p));
                }

                return field;
            }
        };
    }
}



#endif