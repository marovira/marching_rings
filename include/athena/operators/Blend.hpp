#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_BLEND_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_BLEND_HPP

#pragma once

#include "Operators.hpp"
#include "ImplicitOperator.hpp"

namespace athena
{
    namespace operators
    {
        class Blend : public ImplicitOperator
        {
        public:
            Blend() = default;
            ~Blend() = default;

            atlas::utils::BBox getBBox() const override
            {
                atlas::utils::BBox box;
                for (auto& f : mFields)
                {
                    box = atlas::utils::join(box, f->getBBox());
                }

                return box;
            }

            atlas::math::Normal grad(atlas::math::Point const& p) const override
            {
                atlas::math::Normal gradient;
                for (auto& f : mFields)
                {
                    gradient += f->grad(p);
                }

                return gradient;
            }

            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u, float offset) const override
            {
                std::vector<atlas::math::Point> result;
                for (auto& f : mFields)
                {
                    auto seeds = f->getSeeds(u, offset);
                    result.insert(result.end(), seeds.begin(), seeds.end());
                }

                return result;
            }

        private:
            float sdf(atlas::math::Point const& p) const override
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