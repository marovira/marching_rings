#ifndef MR_INCLUDE_MR_CORE_LOG_HPP
#define MR_INCLUDE_MR_CORE_LOG_HPP

#pragma once

#include "Macros.hpp"
#include "Platform.hpp"

#include <string>


namespace mr
{
    namespace core
    {
        namespace Log
        {
            enum class SeverityLevel : int
            {
                Debug = 0,
                Info,
                Warning,
                Error,
                Critical
            };

            void log(SeverityLevel level, std::string const& message);

            void log(SeverityLevel level, const char* format, ...);
        }
    }
}

#define LOG(level, message) \
    mr::core::Log::log(level, message)

#define LOG_V(level, format, ...) \
    mr::core::Log::log(level, format, __VA_ARGS__)

#if defined(MR_DEBUG)
#define DEBUG_LOG(message) \
    LOG(mr::core::Log::SeverityLevel::Debug, message)

#define DEBUG_LOG_V(format, ...) \
    LOG_V(mr::core::Log::SeverityLevel::Debug, format, __VA_ARGS__)
#endif

#define INFO_LOG(message) \
    LOG(mr::core::Log::SeverityLevel::Info, message)

#define INFO_LOG_V(format, ...) \
    LOG_V(mr::core::Log::SeverityLevel::Info, format, __VA_ARGS__)

#define WARN_LOG(message) \
    LOG(mr::core::Log::SeverityLevel::Warning, message)

#define WARN_LOG_V(format, ...) \
    LOG_V(mr::core::Log::SeverityLevel::Warning, format, __VA_ARGS__)

#define ERROR_LOG(message) \
    LOG(mr::core::Log::SeverityLevel::Error, message)

#define ERROR_LOG_V(format, ...) \
    LOG_V(mr::core::Log::SeverityLevel::Error, format, __VA_ARGS__)

#define CRITICAL_LOG(message) \
    LOG(mr::core::Log::SeverityLevel::Critical, message)

#define CRITICAL_LOG_V(format, ...) \
    LOG(mr::core::Log::SeverityLevel::Critical, format, __VA_ARGS__)

#endif