#pragma once
#include <corecrt_io.h>

#define popen _popen
#define pclose _pclose
#define pipe(a) _pipe(a, sizeof(a) / sizeof(a[0]), 0)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define S_IFIFO 0
#define S_IFBLK 0

#define S_ISDIR(a) (a & S_IFDIR)
