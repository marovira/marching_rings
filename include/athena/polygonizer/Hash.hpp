#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_HASH_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_HASH_HPP

#pragma once

#include <cinttypes>

namespace athena
{
    namespace polygonizer
    {
        template <typename T>
        struct BsoidHash
        {
            static constexpr T bits = std::numeric_limits<T>::digits / 2;
            static constexpr T mask = ~(static_cast<T>(0) << bits);
            static constexpr T hash(T x, T y)
            {
                return ((x & mask) << bits | ((y & mask));
            }
        };

        using BsoidHash32 = BsoidHash<std::uint32_t>;
        using BsoidHash64 = BsoidHash<std::uint64_t>;
    }
}

#endif