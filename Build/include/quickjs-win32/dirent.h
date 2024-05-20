#pragma once
#include <direct.h>

typedef int DIR;
struct dirent { const char* d_name; };

static DIR* opendir(const char* __path)
{
    return NULL;
}

static struct dirent* readdir(DIR* __dir)
{
    return NULL;
}

static int closedir(DIR* __dir)
{
    return 0;
}
