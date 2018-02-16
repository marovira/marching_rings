#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_OPERATORS_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_OPERATORS_HPP

#pragma once

#include "athena/Athena.hpp"

#include <memory>

namespace athena
{
    namespace operators
    {
        template <std::size_t Arity>
        class ImplicitOperator;

        template <std::size_t Arity>
        class Blend;

        template <std::size_t Arity>
        class Union;

        template <std::size_t Arity>
        class Intersection;

        template <std::size_t Arity>
        using ImplicitOperatorPtr = std::shared_ptr<ImplicitOperator<Arity>>;
    }
}

#endif