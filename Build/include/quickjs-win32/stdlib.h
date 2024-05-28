#pragma once
#pragma warning(disable:4018)
#pragma warning(disable:4146)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4334)
#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <../ucrt/stdlib.h>
#define alloca _alloca

#ifndef __clang__
#define __builtin_expect(a, b) a
#define __attribute(...)
#define __attribute__(...)
#if defined(_M_IX86)
#define __builtin_clz _lzcnt_u32
#define __builtin_ctz _tzcnt_u32
#define __builtin_clzll(x) ((int)(x >> 32)) ? _lzcnt_u32((int)(x >> 32)) : _lzcnt_u32((int)(x      )) + 32
#define __builtin_ctzll(x) ((int)(x      )) ? _tzcnt_u32((int)(x      )) : _tzcnt_u32((int)(x >> 32)) + 32
#elif defined(_M_AMD64)
#define __builtin_clz _lzcnt_u32
#define __builtin_ctz _tzcnt_u32
#define __builtin_clzll _lzcnt_u64
#define __builtin_ctzll _tzcnt_u64
#else
#define __builtin_clz _CountLeadingZeros
#define __builtin_ctz _CountTrailingZeros
#define __builtin_clzll _CountLeadingZeros64
#define __builtin_ctzll _CountTrailingZeros64
#endif
#define __builtin_frame_address(...) _ReturnAddress()

// It maybe some definitions in the Visual C++
#include <math.h>
inline double ceild(double x) { return ceil(x); }
inline double floord(double x) { return floor(x); }
inline double log2d(double x) { return log2(x); }
#define ceil ceild
#define floor floord
#define log2 log2d
#endif
