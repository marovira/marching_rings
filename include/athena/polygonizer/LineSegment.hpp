#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_LINE_SEGMENT_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_LINE_SEGMENT_HPP

#pragma once

#include "Voxel.hpp"

namespace athena
{
    namespace polygonizer
    {
        struct LinePoint
        {
            LinePoint()
            { }

            LinePoint(FieldPoint const& p) :
                point(p)
            { }

            LinePoint(FieldPoint const& p, std::uint64_t const& e) :
                point(p),
                edge(e)
            { }

            FieldPoint point;
            std::uint64_t edge;
        };

        struct LineSegment
        {
            LineSegment(LinePoint const& s, LinePoint const& e) :
                start(s),
                end(e)
            { }

            LinePoint start, end;
        };
    }
}

#endif