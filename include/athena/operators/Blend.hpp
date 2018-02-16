#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_BLEND_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_BLEND_HPP

#pragma once

#include "Operators.hpp"
#include "ImplicitOperator.hpp"

namespace athena
{
    namespace operators
    {
        template <std::size_t Arity>
        class Blend : public ImplicitOperator<Arity>
        {
        public:
            Blend() = default;
            ~Blend() = default;

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
                atlas::math::Normal gradient;
                for (auto& f : mFields)
                {
                    gradient += f->grad(p);
                }

                return gradient;
            }

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u) const
            {
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
                float field = 0.0f;
                for (auto& f : mFields)
                {
                    field += f->eval(p);
                }

                return field;
            }
        };
    }
}

#endif