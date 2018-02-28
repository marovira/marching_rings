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

        inline float softObjects(float d)
        {
            constexpr float A = 4.0f / 9.0f;
            constexpr float B = 17.0f / 9.0f;
            constexpr float C = 22.0f / 9.0f;

            if (d < 0.0f)
            {
                return 1.0f;
            }

            if (d > 1.0f)
            {
                return 0.0f;
            }

            float d2 = d * d;
            float d4 = d2 * d2;
            float d6 = d4 * d2;

            return (1.0f - A * d6 + B * d4 - C * d2);
        }
    }
}

#endif