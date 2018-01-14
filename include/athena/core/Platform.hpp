#ifndef ATHENA_INCLUDE_ATHENA_CORE_PLATFORM_HPP
#define ATHENA_INCLUDE_ATHENA_CORE_PLATFORM_HPP

#pragma once

/**
* \def ATHENA_PLATFORM_WINDOWS
* This macro is enabled whenever the code is being built on a Windows
* machine regardless of the bitness of the application. This is used
* for code that should run on all Windows configurations.
*
* \def ATHENA_PLATFORM_WIN32
* This is defined for 32 bit applications compiled in Windows. This is
* used whenever 32-bit specific operations need to be performed.
*
* \def ATHENA_PLATFORM_WIN64
* This is defined for 64 bit applications compiled in Windows. This is
* used whenever 64-bit specific operations need to be performed.
*/
#if defined(_WIN32) || defined(_WIN64)
#define ATHENA_PLATFORM_WINDOWS
#if defined(_WIN32)
#define ATHENA_PLATFORM_WIN32
#elif defined(_WIN64)
#define ATHENA_PLATFORM_WIN64
#endif

/**
* \def MR_PLATFORM_LINUX
* This macro is enabled on all Linux platforms (independent of the distro
* being used). This is used for Linux specific code.
*/
#elif defined(__linux__)
#define ATHENA_PLATFORM_LINUX
#endif


#endif