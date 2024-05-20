//==============================================================================
// Minamoto : QuickJS Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <unistd.h>
#include <queue>
#include <string>
#include "QuickJS.h"

#if defined(_WIN32)
typedef intptr_t ssize_t;
#endif

extern "C"
{
#ifdef __clang__
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>
}

JSRuntime* QuickJS::rt;
JSContext* QuickJS::ctx;
std::deque<char> QuickJS::Inputs;
std::deque<std::string> QuickJS::Outputs;
//==============================================================================
extern "C" void quickjs_exit(int num)
{
    QuickJS::Outputs.back().append("exit");
    QuickJS::Outputs.back().append(" ");
    QuickJS::Outputs.back().append(std::to_string(num));
    QuickJS::Outputs.push_back(std::string());
}
//------------------------------------------------------------------------------
extern "C" ssize_t quickjs_read(int fd, void* ptr, size_t nbytes)
{
    if (fd == STDIN_FILENO)
    {
        char* p = (char*)ptr;
        size_t count = std::min(nbytes, QuickJS::Inputs.size());
        for (size_t i = 0; i < count; ++i)
        {
            uint8_t c = QuickJS::Inputs.front();
            QuickJS::Inputs.pop_front();
            p[i] = c;
        }
        return count;
    }
    return read(fd, ptr, nbytes);
}
//------------------------------------------------------------------------------
extern "C" ssize_t quickjs_write(int fd, void const* ptr, size_t nbytes)
{
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        bool csi = false;
        char const* str = (char*)ptr;
        size_t count = nbytes;
        for (size_t i = 0; i < count; ++i)
        {
            char c = str[i];
            if (csi)
            {
                if (c == 'D')
                {
                    size_t count = 0;
                    for (ssize_t j = i - 1; j >= 0; --j)
                    {
                        char c = str[j];
                        if (c < '0' || c > '9')
                            break;
                        count = count * 10 + (c - '0');
                    }
                    if (count == 0)
                        count = 1;
                    for (size_t i = 0; i < count; ++i)
                    {
                        if (QuickJS::Outputs.back().empty())
                            break;
                        QuickJS::Outputs.back().pop_back();
                    }
                }
                if (c >= 'A' && c <= 'Z')
                    csi = false;
                if (c >= 'a' && c <= 'z')
                    csi = false;
                continue;
            }
            if (c == '\x1b')
            {
                csi = true;
                continue;
            }
            if (c == '\r' || c == '\n')
            {
                QuickJS::Outputs.push_back(std::string());
                continue;
            }
            QuickJS::Outputs.back().push_back(c);
        }
        return count;
    }
    return write(fd, ptr, nbytes);
}
//------------------------------------------------------------------------------
extern "C" int quickjs_putchar(int c)
{
    quickjs_write(STDOUT_FILENO, &c, 1);
    return 0;
}
//------------------------------------------------------------------------------
extern "C" int quickjs_fgetc(FILE* stream)
{
    if (stream == stdin)
    {
        if (QuickJS::Inputs.empty())
            return EOF;
        uint8_t c = QuickJS::Inputs.front();
        QuickJS::Inputs.pop_front();
        return c;
    }
    return fgetc(stream);
}
//------------------------------------------------------------------------------
extern "C" size_t quickjs_fread(void* ptr, size_t size, size_t nitems, FILE* stream)
{
    if (stream == stdin)
        return quickjs_read(STDIN_FILENO, ptr, size * nitems);
    return fread(ptr, size, nitems, stream);
}
//------------------------------------------------------------------------------
extern "C" size_t quickjs_fwrite(void const* ptr, size_t size, size_t nitems, FILE* stream)
{
    if (stream == stdout || stream == stderr)
        return quickjs_write(STDOUT_FILENO, ptr, size * nitems);
    return fwrite(ptr, size, nitems, stream);
}
//------------------------------------------------------------------------------
extern "C" bool quickjs_stdin;
extern "C" int quickjs_poll(JSContext* ctx);
//------------------------------------------------------------------------------
void QuickJS::Initialize()
{
    Outputs.push_back(std::string());

    rt = JS_NewRuntime();
    js_std_set_worker_new_context_func(JS_NewContext);
    js_std_init_handlers(rt);
    ctx = JS_NewContext(rt);

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader, nullptr);
}
//------------------------------------------------------------------------------
void QuickJS::StandardLibrary()
{
    /* system modules */
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    /* console modules */
    js_std_add_helpers(ctx, 0, nullptr);
}
//------------------------------------------------------------------------------
void QuickJS::Shutdown()
{
    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    Inputs = std::deque<char>();
    Outputs = std::deque<std::string>();
}
//------------------------------------------------------------------------------
void QuickJS::Input(char c)
{
    Inputs.push_back(c);
    quickjs_stdin = true;
}
//------------------------------------------------------------------------------
void QuickJS::Eval(uint8_t const* buf, size_t len)
{
    js_std_eval_binary(ctx, buf, len, 0);
}
//------------------------------------------------------------------------------
void QuickJS::Update()
{
    quickjs_poll(ctx);

    JSContext* ctx1;
    int err = JS_ExecutePendingJob(rt, &ctx1);
    if (err < 0)
    {
        js_std_dump_error(ctx1);
        return;
    }
}
//==============================================================================
