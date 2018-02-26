#ifndef ATHENA_INCLUDE_ATHENA_OPERATORS_OPERATORS_HPP
#define ATHENA_INCLUDE_ATHENA_OPERATORS_OPERATORS_HPP

#pragma once

#include "athena/Athena.hpp"

#include <memory>

namespace athena
{
    namespace operators
    {
        class ImplicitOperator;

        class Blend;
        class Union;
        class Intersection;

        using ImplicitOperatorPtr = std::shared_ptr<ImplicitOperator>;
    }
}

#endif