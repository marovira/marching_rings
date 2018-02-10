#ifndef ATHENA_INCLUDE_ATHENA_BLOB_TABLES_HPP
#define ATHENA_INCLUDE_ATHENA_BLOB_TABLES_HPP

#pragma once

#include <atlas/math/Math.hpp>

#include <vector>
#include <map>

namespace athena
{
    namespace polygonizer
    {
        const std::vector<glm::u32vec2> VoxelDecals = 
        {
            { 0, 0 },
            { 1, 0 },
            { 1, 1 },
            { 0, 1 }
        };

        const std::map<int, glm::ivec2> EdgeDecals = 
        {
            { 0, {  0, -1 } },
            { 1, {  1,  0 } },
            { 2, {  0,  1 } },
            { 3, { -1,  0 } }
        };

        constexpr unsigned int EdgeTable[16] =
        {
            0x00, 0x09, 0x03, 0x0A,
            0x06, 0x0F, 0x05, 0x0C,
            0x0C, 0x05, 0x0F, 0x06,
            0x0A, 0x03, 0x09, 0x00
        };

        constexpr int LineTable[16][4]
        {
            { -1, -1, -1, -1 }, // 0
            {  3,  0, -1, -1 }, // 1
            {  0,  1, -1, -1 }, // 2
            {  3,  1, -1, -1 }, // 3
            {  1,  2, -1, -1 }, // 4
            {  1,  0,  3,  2 }, // 5
            {  0,  2, -1, -1 }, // 6
            {  3,  2, -1, -1 }, // 7
            {  2,  3, -1, -1 }, // 8
            {  2,  0, -1, -1 }, // 9
            {  0,  3,  2,  1 }, // 10
            {  2,  1, -1, -1 }, // 11
            {  1,  3, -1, -1 }, // 12
            {  1,  0, -1, -1 }, // 13
            {  0,  3, -1, -1 }, // 14
            { -1, -1, -1, -1 }  // 15
        };

    }
}

#endif