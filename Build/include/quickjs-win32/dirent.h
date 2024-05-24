#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

struct dirent
{
    const char* d_name;
};

typedef struct DIR
{
    WIN32_FIND_DATAA data;
    HANDLE handle;
    struct dirent dirent;
} DIR;

static DIR* opendir(const char* __path)
{
    DIR* dir = calloc(1, sizeof(DIR));
    if (dir == NULL) {
        return NULL;
    }
    char temp[MAX_PATH];
    strcpy_s(temp, MAX_PATH, __path);
    size_t length = strlen(__path);
    if (length >= 1) {
        char c = __path[length - 1];
        if (c != '*') {
            if (c != '/' && c != '\\') {
                strcat_s(temp, MAX_PATH, "/*");
            }
            else {
                strcat_s(temp, MAX_PATH, "*");
            }
        }
    }
    dir->handle = FindFirstFileA(temp, &dir->data);
    if (dir->handle == 0 || dir->handle == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    return dir;
}

static struct dirent* readdir(DIR* __dir)
{
    if (__dir == NULL) {
        return NULL;
    }
    else if (__dir->dirent.d_name == NULL) {
        __dir->dirent.d_name = __dir->data.cFileName;
    }
    else if (FindNextFileA(__dir->handle, &__dir->data) == FALSE) {
        return NULL;
    }
    return &__dir->dirent;
}

static int closedir(DIR* __dir)
{
    if (__dir == NULL) {
        return 0;
    }
    FindClose(__dir->handle);
    free(__dir);
    return 0;
}
