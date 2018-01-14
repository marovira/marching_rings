#include "athena/core/Log.hpp"

#if defined(ATHENA_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdarg>

namespace athena
{
    namespace core
    {
        namespace Log
        {
            static const int kMaxLogLength = 16 * 1024;
            static const std::vector<std::string> kLevelStrings =
                std::vector<std::string>
            {
                "debug",
                "info",
                "warning",
                "error",
                "critical"
            };

            void _log(std::string const& message)
            {
                const int maxLogLength = 16 * 1024;
                char buf[maxLogLength];
                memcpy(buf, message.c_str(), message.size() + 1);

#if defined(ATHENA_PLATFORM_WINDOWS)
                strncat_s(buf, "\n", 3);

                WCHAR wszBuf[maxLogLength] = { 0 };
                MultiByteToWideChar(CP_UTF8, 0, buf, -1, wszBuf, sizeof(wszBuf));
                OutputDebugStringW(wszBuf);
                WideCharToMultiByte(CP_ACP, 0, wszBuf, -1, buf, sizeof(buf),
                    nullptr, FALSE);
                printf("%s", buf);
                fflush(stdout);
#else
                strncat(buf, "\n", 3);
                fprintf(stdout, "%s", buf);
                fflus(stdout);
#endif
            }

            std::string getTimeStamp()
            {
                auto now = std::chrono::system_clock::now();
                auto nowTime = std::chrono::system_clock::to_time_t(now);

                std::stringstream stream;
                stream << std::put_time(std::localtime(&nowTime), "%T");
                return stream.str();
            }

            void log(Log::SeverityLevel level, std::string const& message)
            {
                const std::vector<std::string> levelStrings = 
                {
                    "debug",
                    "info",
                    "warning",
                    "error",
                    "critical"
                };

                std::string logMessage = "";

                // Get the current time stamp.
                logMessage.append(getTimeStamp());
                logMessage.append("    ");

                std::string sevLevel = "[";
                int levelNum = static_cast<int>(level);
                sevLevel.append(levelStrings[levelNum]);
                sevLevel.append("] : ");

                logMessage.append(sevLevel);
                logMessage.append(message);
                _log(logMessage);

            }

            void log(Log::SeverityLevel level, const char* format, ...)
            {
                const int maxLogLength = 16 * 1024;

                char buffer[maxLogLength];
                va_list args;
                va_start(args, format);
                vsprintf(buffer, format, args);
                va_end(args);

                log(level, std::string(buffer));
            }

        }
    }
}
