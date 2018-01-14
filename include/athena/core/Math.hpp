#ifndef MR_INCLUDE_MR_CORE_MATH_HPP
#define MR_INCLUDE_MR_CORE_MATH_HPP

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtc/constants.hpp>

namespace mr
{
    namespace core
    {
        using Vector2 = glm::vec2;
        using Point2 = Vector2;

        using Vector = glm::vec3;
        using Point = Vector;
        using Normal = Vector;

        using Vector4 = glm::vec4;
        using Point4 = Vector4;
    }
}


#endif