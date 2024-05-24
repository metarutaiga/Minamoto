#!/bin/sh
# ==============================================================================
# Minamoto : quickjs Bash
# 
# Copyright (c) 2019-2024 TAiGA
# https://github.com/metarutaiga/minamoto
# ==============================================================================

if [ ! -f quickjs-ver.h ] || [ VERSION -nt quickjs-ver.h ]; then
  echo \#define CONFIG_VERSION \"$(cat VERSION)\" > quickjs-ver.h
fi
if [ ! -f qjsc ]; then
  clang -DCONFIG_VERSION=\"$(cat VERSION)\" -Ofast cutils.c libbf.c libregexp.c libunicode.c qjsc.c quickjs.c quickjs-libc.c -o qjsc
fi
if [ ! -f qjscalc.c ]; then
  ./qjsc -c -o qjscalc.c -m qjscalc.js
fi
if [ ! -f repl.c ]; then
  ./qjsc -c -o repl.c -m repl.js
fi
