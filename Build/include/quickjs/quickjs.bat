@echo off
rem // ==============================================================================
rem // Minamoto : quickjs Batch
rem //
rem // Copyright (c) 2019-2024 TAiGA
rem // https://github.com/metarutaiga/minamoto
rem //==============================================================================

if not exist qjsc.exe (
  set /P CONFIG_VERSION=<VERSION
  cl -std:c11 -experimental:c11atomics -I..\..\Build\include\quickjs-win32 -DCONFIG_BIGNUM -DCONFIG_VERSION="%CONFIG_VERSION%" -Ox cutils.c libbf.c libregexp.c libunicode.c qjsc.c quickjs.c quickjs-libc.c -o qjsc.exe
)
if not exist qjscalc.c (
  qjsc.exe -c -o qjscalc.c -m qjscalc.js
)
if not exist repl.c (
  qjsc.exe -c -o repl.c -m repl.js
)
