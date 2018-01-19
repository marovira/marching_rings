#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_FILTERS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_FILTERS_HPP

#pragma once

namespace athena
{
    namespace fields
    {
        inline float wyvill(float d)
        {
            if (d < 0.0f)
            {
                return 1.0f;
            }
            else if (d > 1.0f)
            {
                return 0.0f;
            }
            else
            {
                float d2 = d * d;
                float base = 1.0f - d2;
                return base * base * base;
            }
        }

        inline float global(float d)
        {
            return d;
        }
    }
}

#endif