@echo off
rem // ==============================================================================
rem // Minamoto : quickjs Batch
rem //
rem // Copyright (c) 2019-2024 TAiGA
rem // https://github.com/metarutaiga/minamoto
rem //==============================================================================

if not exist qjsc.exe (
  set /P CONFIG_VERSION=<VERSION
  clang -I..\..\Build\include\quickjs-win32 -Dalloca=_alloca -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DCONFIG_VERSION="%CONFIG_VERSION%" -Ofast ..\ClangPlatform\compiler-rt\udivmodti4.c ..\ClangPlatform\compiler-rt\udivti3.c cutils.c libbf.c libregexp.c libunicode.c qjsc.c quickjs.c quickjs-libc.c -o qjsc.exe
)
if not exist qjscalc.c (
  qjsc.exe -c -o qjscalc.c -m qjscalc.js
)
if not exist repl.c (
  qjsc.exe -c -o repl.c -m repl.js
)
