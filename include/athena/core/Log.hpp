#ifndef ATHENA_INCLUDE_ATHENA_CORE_LOG_HPP
#define ATHENA_INCLUDE_ATHENA_CORE_LOG_HPP

#pragma once

#include "Macros.hpp"
#include "Platform.hpp"

#include <string>


namespace athena
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
    athena::core::Log::log(level, message)

#define LOG_V(level, format, ...) \
    athena::core::Log::log(level, format, __VA_ARGS__)

#if defined(ATHENA_DEBUG)
#define DEBUG_LOG(message) \
    LOG(athena::core::Log::SeverityLevel::Debug, message)

#define DEBUG_LOG_V(format, ...) \
    LOG_V(athena::core::Log::SeverityLevel::Debug, format, __VA_ARGS__)
#else
#define DEBUG_LOG(message)

#define DEBUG_LOG_V(format, ...)
#endif

#define INFO_LOG(message) \
    LOG(athena::core::Log::SeverityLevel::Info, message)

#define INFO_LOG_V(format, ...) \
    LOG_V(athena::core::Log::SeverityLevel::Info, format, __VA_ARGS__)

#define WARN_LOG(message) \
    LOG(athena::core::Log::SeverityLevel::Warning, message)

#define WARN_LOG_V(format, ...) \
    LOG_V(athena::core::Log::SeverityLevel::Warning, format, __VA_ARGS__)

#define ERROR_LOG(message) \
    LOG(athena::core::Log::SeverityLevel::Error, message)

#define ERROR_LOG_V(format, ...) \
    LOG_V(athena::core::Log::SeverityLevel::Error, format, __VA_ARGS__)

#define CRITICAL_LOG(message) \
    LOG(athena::core::Log::SeverityLevel::Critical, message)

#define CRITICAL_LOG_V(format, ...) \
    LOG(athena::core::Log::SeverityLevel::Critical, format, __VA_ARGS__)

#endif