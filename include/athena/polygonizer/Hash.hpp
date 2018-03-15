#ifndef ATHENA_INCLUDE_ATHENA_POLYGONIZER_HASH_HPP
#define ATHENA_INCLUDE_ATHENA_POLYGONIZER_HASH_HPP

#pragma once

#include <atlas/core/Macros.hpp>

#include <cinttypes>
#include <limits>

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
                return ((x & mask) << bits | (y & mask));
            }
        };

        using BsoidHash32 = BsoidHash<std::uint32_t>;
        using BsoidHash64 = BsoidHash<std::uint64_t>;

#ifdef ATLAS_DEBUG
#include <atlas/math/Math.hpp>
#include <tuple>

        glm::u32vec2 reverseHash32(std::uint32_t hash)
        {
            std::uint32_t mask = 0xFFFF;
            std::uint32_t y = hash & mask;
            std::uint32_t x = (hash & (mask << 16)) >> 16;
            return { x, y };
        }

        std::tuple<glm::u32vec2, glm::u32vec2> reverseHash64(std::uint64_t hash)
        {
            std::uint64_t mask = 0xFFFFFFFF;
            std::uint32_t u = hash & mask;
            std::uint32_t v = (hash & (mask << 32)) >> 32;

            return { reverseHash32(u), reverseHash32(v) };
        }
#endif
    }
}

#endif