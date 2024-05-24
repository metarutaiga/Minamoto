#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <corecrt_io.h>

typedef intptr_t ssize_t;

#define getpid GetCurrentProcessId
#define popen _popen
#define pclose _pclose
#define pipe(a) _pipe(a, sizeof(a) / sizeof(a[0]), 0)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define S_IFIFO 0
#define S_IFBLK 0
#define S_ISDIR(a) (a & S_IFDIR)

static const char* optarg = "";
static int optind = 1;
static int getopt(int argc, char** argv, const char* optstring)
{
    if (optind < argc) {
        char* arg = argv[optind];
        if (arg[0] == '-') {
            optind++;
            for (char c; (c = *optstring); ++optstring) {
                if (arg[1] == c) {
                    if (optstring[1] == ':') {
                        optarg = argv[optind++];
                    }
                    return c;
                }
            }
            return '?';
        }
    }
    return -1;
}
