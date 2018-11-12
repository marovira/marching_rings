#ifndef ATHENA_INCLUDE_ATHENA_MODELS_MODELS_HPP
#define ATHENA_INCLUDE_ATHENA_MODELS_MODELS_HPP

#pragma once

#include "athena/polygonizer/Bsoid.hpp"
#include "athena/polygonizer/MarchingCubes.hpp"

#include <functional>

#define MAKE_SOID_FUNCTION(name) \
athena::polygonizer::Bsoid make##name()

#define MAKE_MC_FUNCTION(name) \
athena::polygonizer::MarchingCubes makeMC##name()


namespace athena
{
    namespace models
    {
        using ModelFn = std::function<athena::polygonizer::Bsoid()>;
        using MCModelFn = std::function<athena::polygonizer::MarchingCubes()>;

        MAKE_SOID_FUNCTION(Sphere);
        MAKE_SOID_FUNCTION(Peanut);
        MAKE_SOID_FUNCTION(Cylinder);
        MAKE_SOID_FUNCTION(Cone);
        MAKE_SOID_FUNCTION(Torus);
        MAKE_SOID_FUNCTION(Chain);

        MAKE_MC_FUNCTION(Sphere);
        MAKE_MC_FUNCTION(Peanut);
        MAKE_MC_FUNCTION(Cylinder);
        MAKE_MC_FUNCTION(Cone);
        MAKE_MC_FUNCTION(Torus);
        MAKE_MC_FUNCTION(Chain);
    }
}

#endif