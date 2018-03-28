#ifndef ATHENA_INCLUDE_ATHENA_FIELDS_FILTERS_HPP
#define ATHENA_INCLUDE_ATHENA_FIELDS_FILTERS_HPP

#pragma once

namespace athena
{
    namespace fields
    {
        static constexpr float radius = 1.0f;

        inline float compactField(float dist)
        {
            if (dist < -radius)
            {
                return 1.0f;
            }
            if (dist > radius)
            {
                return 0.0f;
            }

            const float x = dist / radius;
            const float x3 = x * x * x;
            const float x5 = x3 * x * x;
            static constexpr float A = -3.0f / 16;
            static constexpr float B = 5.0f / 8;
            static constexpr float C = 15.0f / 16.0f;
            static constexpr float D = 0.5f;

            return (A * x5) + (B * x3) - (C * x) + D;
        }

        inline float compactGradient(float dist)
        {
            if (dist < -radius || dist > radius)
            {
                return 0.0f;
            }

            const float x = dist / radius;
            const float x2 = x * x;
            const float x4 = x2 * x2;
            static constexpr float A = -15.0f / 16;
            static constexpr float B = 15.0f / 8;
            static constexpr float C = 15.0f / 16;

            return (A * x4) + (B * x2) - C;

        }
    }
}

#endif