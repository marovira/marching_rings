#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_FIELDS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_FIELDS_HPP

#pragma once

#include "athena/Athena.hpp"

#include <functional>

namespace athena
{
    namespace fields
    {
        using FilterFn = std::function<float(float)>;

        class ImplicitField;
        class Sphere;
        class Torus;
    }
}

#endif