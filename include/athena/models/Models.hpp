#ifndef ATHENA_INCLUDE_ATHENA_MODELS_MODELS_HPP
#define ATHENA_INCLUDE_ATHENA_MODELS_MODELS_HPP

#pragma once

#include "athena/polygonizer/Bsoid.hpp"

#include <functional>

#define MAKE_FUNCTION(name) \
athena::polygonizer::Bsoid make##name()

namespace athena
{
    namespace models
    {
        using ModelFn = std::function<athena::polygonizer::Bsoid()>;

        MAKE_FUNCTION(Sphere);
        MAKE_FUNCTION(Peanut);
        MAKE_FUNCTION(Cylinder);
        MAKE_FUNCTION(Cone);
    }
}

#endif