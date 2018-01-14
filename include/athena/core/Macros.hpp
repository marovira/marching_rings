#ifndef ATHENA_INCLUDE_ATHENA_CORE_MACROS_HPP
#define MR_INLCUDE_MR_CORE_MACROS_HPP

#pragma once

#define UNUSED(expr) (void)expr

#if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG__)
#define ATHENA_DEBUG
#endif

#endif