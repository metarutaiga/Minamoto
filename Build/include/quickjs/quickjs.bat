@echo off
rem // ==============================================================================
rem // Minamoto : quickjs Batch
rem //
rem // Copyright (c) 2019-2024 TAiGA
rem // https://github.com/metarutaiga/minamoto
rem //==============================================================================

for /f %%s in ('dir /b /o:d VERSION quickjs-ver.h') do set newer=%%s
if not %newer%==quickjs-ver.h (
  for /f %%s in (VERSION) do echo #define CONFIG_VERSION "%%s" > quickjs-ver.h
  del qjsc.exe
  del qjscalc.c
  del repl.c
)
if not exist qjsc.exe (
  cl -std:c11 -experimental:c11atomics -I. -I../../Build/include/quickjs-win32 -FI ../../Build/include/quickjs-user.h -Ox cutils.c libbf.c libregexp.c libunicode.c qjsc.c quickjs.c quickjs-libc.c -o qjsc.exe
)
if not exist qjscalc.c (
  qjsc.exe -c -o qjscalc.c -m qjscalc.js
)
if not exist repl.c (
  qjsc.exe -c -o repl.c -m repl.js
)
for /f %%s in ('dir /b /o:d quickjs.h quickjs.hpp') do set newer=%%s
if not %newer%==quickjs.hpp (
  powershell -Command "(Get-Content quickjs.h) -replace '\(JSValue\){ \(JSValueUnion\){', '{ {' | Set-Content quickjs.1"
  powershell -Command "(Get-Content quickjs.1) -replace '.u = {', '{' | Set-Content quickjs.hpp"
  del quickjs.1
)
