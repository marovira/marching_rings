#ifndef MR_INCLUDE_MR_CORE_MACROS_HPP
#define MR_INLCUDE_MR_CORE_MACROS_HPP

#pragma once

#define UNUSED(expr) (void)expr

#if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG__)
#define MR_DEBUG
#endif

#endif