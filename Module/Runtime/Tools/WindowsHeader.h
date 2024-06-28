//==============================================================================
// Minamoto : WindowsHeader Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
typedef float FLOAT;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int16_t SHORT;
typedef int32_t INT;
typedef int32_t LONG;
typedef uint16_t USHORT;
typedef uint32_t UINT;
typedef uint64_t UINT64;
typedef uint64_t LARGE_INTEGER;
typedef void VOID;
typedef long HRESULT;
typedef void* HANDLE;
typedef void* HWND;
typedef void* LPVOID;
typedef DWORD* LPDWORD;
typedef struct GUID { uint32_t a; uint16_t b; uint16_t c; uint8_t d[8]; } GUID;
typedef struct RECT { LONG left; LONG top; LONG right; LONG bottom; } RECT, *LPRECT;
#define FAR
#define PASCAL
#define APIENTRY
#define BOOL int
#define TRUE true
#define FALSE false
#define DEFINE_GUID(...)
#define _Return_type_success_(...)
#define __in
#define ZeroMemory(a,b) memset(a,0,b)
#endif
