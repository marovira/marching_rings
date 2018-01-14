#ifndef MR_INCLUDE_MR_CORE_TIMER_HPP
#define MR_INCLUDE_MR_CORE_TIMER_HPP

#pragma once

#include <chrono>

namespace mr
{
    namespace core
    {
        template <typename GenType>
        class Timer
        {
        public:
            Timer()
            { }

            void start()
            {
                mBegin = Clock::now();
            }

            void reset()
            {
                mBegin = std::chrono::time_point<Clock>();
            }

            GenType elapsed() const
            {
                return std::chrono::duration_cast<Second>(
                    Clock::now() - mBegin).count();
            }

        private:
            using Clock = std::chrono::high_resolution_clock;
            using Second = std::chrono::duration<GenType>;
            std::chrono::time_point<Clock> mBegin;
        };
    }
}

#endif