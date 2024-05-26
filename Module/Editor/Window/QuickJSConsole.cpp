//==============================================================================
// Minamoto : QuickJSConsole Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <unistd.h>
#include <queue>
#include <string>
#include "Script/QuickJS.h"
#include "Component/Console.h"
#include "QuickJSConsole.h"

extern "C"
{
#include <quickjs/quickjs-ver.h>
#include "../../Build/include/quickjs-user.h"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#include <quickjs/quickjs.h>
#elif defined(_MSC_VER)
#include <quickjs/quickjs.hpp>
#endif
#include <quickjs/quickjs-libc.h>
#include <quickjs/repl.c>
}

static Console console;
static std::deque<char> Inputs;
static std::deque<std::string> Outputs;
//------------------------------------------------------------------------------
extern "C" bool quickjs_stdin;
extern "C" int quickjs_poll(JSContext*);
//------------------------------------------------------------------------------
void QuickJSConsole::Initialize()
{
    Outputs.push_back(std::string());

    QuickJS::dump_error = js_std_dump_error;

    js_std_set_worker_new_context_func(JS_NewContext);
    js_std_init_handlers(QuickJS::rt);

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(QuickJS::rt, nullptr, js_module_loader, nullptr);

    /* system modules */
    js_init_module_std(QuickJS::ctx, "std");
    js_init_module_os(QuickJS::ctx, "os");

    /* console modules */
    js_std_add_helpers(QuickJS::ctx, 0, nullptr);

    js_std_eval_binary(QuickJS::ctx, qjsc_repl, qjsc_repl_size, 0);
}
//------------------------------------------------------------------------------
void QuickJSConsole::Shutdown()
{
    js_std_free_handlers(QuickJS::rt);

    Inputs = std::deque<char>();
    Outputs = std::deque<std::string>();
}
//------------------------------------------------------------------------------
bool QuickJSConsole::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    bool update = false;
    if (ImGui::Begin(ICON_FA_TABLET "QuickJS Console", &show))
    {
        if (ImGui::IsWindowFocused())
        {
            update |= console.UpdateInput(updateData);
        }

        bool appearing = ImGui::IsWindowAppearing();
        if (appearing || ImGui::IsKeyReleased(ImGuiKey_Enter))
        {
            auto& input = console.input;
            auto& inputPos = console.inputPos;

            char c = 0;
            if (appearing && input.empty() == false)
                std::swap(input[0], c);

            if (input.empty() == false && input[0] != 0)
            {
                for (char c : input)
                {
                    Inputs.push_back(c);
                }
                Inputs.push_back('\n');
                console.AddHistory(input.c_str());
                input.clear();
                inputPos = 0;
                update = true;

                // Update
                quickjs_stdin = true;
                quickjs_poll(QuickJS::ctx);
                QuickJS::Update();
            }

            if (appearing && input.empty() == false)
                std::swap(input[0], c);
        }

        auto const& lines = Outputs;
        update |= console.UpdateConsole(updateData, lines);
    }
    ImGui::End();

    return update;
}
//==============================================================================
//  Standard I/O
//==============================================================================
extern "C" void quickjs_exit(int num)
{
    Outputs.back().append("exit");
    Outputs.back().append(" ");
    Outputs.back().append(std::to_string(num));
    Outputs.push_back(std::string());
}
//------------------------------------------------------------------------------
extern "C" ssize_t quickjs_read(int fd, void* ptr, size_t nbytes)
{
    if (fd == STDIN_FILENO)
    {
        char* p = (char*)ptr;
        size_t count = std::min(nbytes, Inputs.size());
        for (size_t i = 0; i < count; ++i)
        {
            uint8_t c = Inputs.front();
            Inputs.pop_front();
            p[i] = c;
        }
        return count;
    }
    return read(fd, ptr, nbytes);
}
//------------------------------------------------------------------------------
extern "C" ssize_t quickjs_write(int fd, void const* ptr, size_t nbytes) __attribute__((optnone))
{
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        bool csi = false;
        char const* str = (char*)ptr;
        size_t count = nbytes;
        for (size_t i = 0; i < count; ++i)
        {
            char c = str[i];
            if (c == 0)
            {
                break;
            }
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
                        if (Outputs.back().empty())
                            break;
                        Outputs.back().pop_back();
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
                Outputs.push_back(std::string());
                continue;
            }
            Outputs.back().push_back(c);
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
extern "C" int quickjs_fileno(FILE* stream)
{
    if (stream == stdin)
        return STDIN_FILENO;
    if (stream == stdout)
        return STDOUT_FILENO;
    if (stream == stderr)
        return STDERR_FILENO;
    return fileno(stream);
}
//------------------------------------------------------------------------------
extern "C" int quickjs_fgetc(FILE* stream)
{
    if (stream == stdin)
    {
        if (Inputs.empty())
            return EOF;
        uint8_t c = Inputs.front();
        Inputs.pop_front();
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
    if (stream == stdout)
        return quickjs_write(STDOUT_FILENO, ptr, size * nitems);
    if (stream == stderr)
        return quickjs_write(STDERR_FILENO, ptr, size * nitems);
    return fwrite(ptr, size, nitems, stream);
}
//------------------------------------------------------------------------------
extern "C" int quickjs_fprintf(FILE* stream, char const* format, ...)
{
    va_list va;
    va_start(va, format);
    if (stream == stdout || stream == stderr)
    {
        int length = vsnprintf(nullptr, 0, format, va);
        char* buffer = xxAlloc(char, length + 1);
        vsnprintf(buffer, length + 1, format, va);
        quickjs_fwrite(buffer, 1, length, stream);
        xxFree(buffer);
        va_end(va);
        return length;
    }
    int result = vfprintf(stream, format, va);
    va_end(va);
    return result;
}
//==============================================================================
