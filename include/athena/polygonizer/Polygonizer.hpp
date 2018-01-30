#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_POLYGONIZER_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_POLYGONIZER_HPP

#pragma once

#include <memory>

namespace athena
{
    namespace polygonizer
    {
        enum class SlicingAxes : int
        {
            XAxis = 0,
            YAxis,
            ZAxis
        };

        class Bsoid;
        class CrossSection;

        using CrossSectionPointer = std::unique_ptr<CrossSection>;
    }
}

#endif