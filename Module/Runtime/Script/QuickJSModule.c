//==============================================================================
// Minamoto : QuickJSModule Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#ifdef __clang__
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wsometimes-uninitialized"
#endif

#if defined(_WIN32)
#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#define USE_WORKER
#endif

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#if defined(_WIN32)
typedef intptr_t ssize_t;
#endif

void    quickjs_exit(int);
ssize_t quickjs_read(int, void*, size_t);
ssize_t quickjs_write(int, void const*, size_t);
int     quickjs_putchar(int);
int     quickjs_fgetc(FILE*);
size_t  quickjs_fread(void*, size_t, size_t, FILE*);
size_t  quickjs_fwrite(void const*, size_t, size_t, FILE*);

#define exit quickjs_exit
#define read quickjs_read
#define write quickjs_write
#define putchar quickjs_putchar
#define fgetc quickjs_fgetc
#define fread quickjs_fread
#define fwrite quickjs_fwrite
#define isatty(a) false
#define select(a,b,c,d,e) select(a,b,c,d,&(struct timeval){0})
#include <quickjs/quickjs-libc.c>

bool quickjs_stdin = false;
int quickjs_poll(JSContext *ctx)
{
    if (quickjs_stdin) {
        quickjs_stdin = false;
        JSRuntime *rt = JS_GetRuntime(ctx);
        JSThreadState *ts = JS_GetRuntimeOpaque(rt);
        JSOSRWHandler *rh;
        struct list_head *el;
        list_for_each(el, &ts->os_rw_handlers) {
            rh = list_entry(el, JSOSRWHandler, link);
            if (!JS_IsNull(rh->rw_func[0]) && rh->fd == STDIN_FILENO) {
                call_handler(ctx, rh->rw_func[0]);
                /* must stop because the list may have been modified */
                goto done;
            }
        }
    }
    done:
    return js_os_poll(ctx);
}
