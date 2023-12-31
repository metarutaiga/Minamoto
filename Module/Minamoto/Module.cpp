//==============================================================================
// Minamoto : Module Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "xxGraphic/xxSystem.h"
#include "DearImGui.h"
#include "Renderer.h"
#include "Logger.h"
#include "Module.h"

static ImVector<void*>                  g_moduleLibraries;
static ImVector<const char*>            g_moduleTitles;
static ImVector<float>                  g_moduleTimers;
static ImVector<PFN_MODULE_CREATE>      g_moduleCreates;
static ImVector<PFN_MODULE_SHUTDOWN>    g_moduleShutdowns;
static ImVector<PFN_MODULE_MESSAGE>     g_moduleMessages;
static ImVector<PFN_MODULE_UPDATE>      g_moduleUpdates;
static ImVector<PFN_MODULE_RENDER>      g_moduleRenders;
//------------------------------------------------------------------------------
void Module::Create(const char* path, uint64_t device)
{
    const char* app = xxGetExecutablePath();
    const char* configuration = "";
    const char* arch = "";
    const char* extension = "";
#if defined(xxWINDOWS)
#if defined(__llvm__)
#elif defined(_DEBUG)
    configuration = "Debug";
#elif defined(NDEBUG)
    configuration = "Release";
#endif
#if defined(__llvm__)
#elif defined(_M_ARM64EC)
    arch = ".arm64ec";
#elif defined(_M_HYBRID_X86_ARM64)
    arch = ".chpe";
#elif defined(_M_AMD64)
    arch = ".x64";
#elif defined(_M_IX86)
    arch = ".Win32";
#elif defined(_M_ARM64)
    arch = ".arm64";
#elif defined(_M_ARM)
    arch = ".arm";
#endif
    extension = ".dll";
#elif defined(xxMACOS)
    extension = ".dylib";
#elif defined(xxIOS)
    extension = ".bundle";
#elif defined(xxANDROID)
    extension = ".so";
#endif

    char temp[4096];
#if defined(xxWINDOWS)
    snprintf(temp, 4096, "%s\\%s", app, path);
#elif defined(xxMACOS) || defined(xxMACCATALYST)
    snprintf(temp, 4096, "%s/../Frameworks", app);
#elif defined(xxIOS)
    snprintf(temp, 4096, "%s/Frameworks", app);
#elif defined(xxANDROID)
    snprintf(temp, 4096, "%s", app);
#endif

    uint64_t handle = 0;
    while (char* filename = xxOpenDirectory(&handle, temp, configuration, arch, extension, nullptr))
    {
#if defined(xxWINDOWS)
        snprintf(temp, 4096, "%s\\%s\\%s", app, path, filename);
#elif defined(xxMACOS) || defined(xxMACCATALYST)
        snprintf(temp, 4096, "%s/../Frameworks/%s", app, filename);
#elif defined(xxIOS)
        snprintf(temp, 4096, "%s/Frameworks/%s", app, filename);
#elif defined(xxANDROID)
        snprintf(temp, 4096, "%s/%s", app, filename);
#endif
        float begin = xxGetCurrentTime();
        void* library = xxLoadLibrary(temp);
        float end = xxGetCurrentTime();
        if (library == nullptr)
        {
            free(filename);
            continue;
        }

        PFN_MODULE_CREATE create = (PFN_MODULE_CREATE)xxGetProcAddress(library, "Create");
        PFN_MODULE_SHUTDOWN shutdown = (PFN_MODULE_SHUTDOWN)xxGetProcAddress(library, "Shutdown");
        PFN_MODULE_MESSAGE message = (PFN_MODULE_MESSAGE)xxGetProcAddress(library, "Message");
        PFN_MODULE_UPDATE update = (PFN_MODULE_UPDATE)xxGetProcAddress(library, "Update");
        PFN_MODULE_RENDER render = (PFN_MODULE_RENDER)xxGetProcAddress(library, "Render");
        if (create == nullptr || shutdown == nullptr || update == nullptr)
        {
            xxFreeLibrary(library);
            free(filename);
            continue;
        }
        free(filename);

        g_moduleLibraries.push_back(library);
        g_moduleCreates.push_back(create);
        g_moduleTimers.push_back(end - begin);
        g_moduleShutdowns.push_back(shutdown);
        if (message) g_moduleMessages.push_back(message);
        g_moduleUpdates.push_back(update);
        if (render) g_moduleRenders.push_back(render);
    }
    xxCloseDirectory(&handle);

    CreateData createData;
    createData.device = device;
    createData.baseFolder = app;
    for (int i = 0; i < g_moduleCreates.size(); ++i)
    {
        PFN_MODULE_CREATE create = g_moduleCreates[i];
        float begin = xxGetCurrentTime();
        const char* title = create(createData);
        float end = xxGetCurrentTime();
        g_moduleTitles.push_back(title);

        g_moduleTimers[i] += end - begin;
        xxLog("Plugin", "Loaded : %s (%.0fus)", title, g_moduleTimers[i] * 1000000);
    }
}
//------------------------------------------------------------------------------
void Module::Shutdown()
{
    ShutdownData shutdownData;
    for (int i = 0; i < g_moduleShutdowns.size(); ++i)
    {
        PFN_MODULE_SHUTDOWN shutdown = g_moduleShutdowns[i];
        shutdown(shutdownData);
    }

    for (int i = 0; i < g_moduleLibraries.size(); ++i)
    {
        void* library = g_moduleLibraries[i];
        xxFreeLibrary(library);
    }

    g_moduleLibraries.clear();
    g_moduleTitles.clear();
    g_moduleCreates.clear();
    g_moduleShutdowns.clear();
    g_moduleMessages.clear();
    g_moduleUpdates.clear();
    g_moduleRenders.clear();
}
//------------------------------------------------------------------------------
int Module::Count()
{
    return (int)g_moduleUpdates.size();
}
//------------------------------------------------------------------------------
void Module::Message(std::initializer_list<const char*> list)
{
    MessageData messageData;
    messageData.data = &(*list.begin());
    messageData.length = list.size();
    if (messageData.length != 0)
    {
        switch (xxHash(messageData.data[0]))
        {
        case xxHash("LOGGER_UPDATE"):
            Logger::Update(*(std::deque<char*>*)messageData.data[1]);
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < g_moduleMessages.size(); ++i)
    {
        PFN_MODULE_MESSAGE message = g_moduleMessages[i];
        message(messageData);
    }
}
//------------------------------------------------------------------------------
bool Module::Update()
{
    bool updated = false;

    static int updateCount = 0;
    if (updateCount)
    {
        updateCount--;
        updated = true;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        updateCount = 1;
    }

    static float previousTime = 0;
    float time = xxGetCurrentTime();
    float elapsed = time - previousTime;
    previousTime = time;

    UpdateData updateData;
    updateData.instance = Renderer::g_instance;
    updateData.device = Renderer::g_device;
    updateData.renderPass = Renderer::g_renderPass;
    updateData.width = Renderer::g_width;
    updateData.height = Renderer::g_height;
    updateData.time = time;
    updateData.elapsed = elapsed;
    updateData.message = Module::Message;
    for (int i = 0; i < g_moduleUpdates.size(); ++i)
    {
        PFN_MODULE_UPDATE update = g_moduleUpdates[i];
        updated |= update(updateData);
    }

    return updated;
}
//------------------------------------------------------------------------------
void Module::Render()
{
    RenderData renderData;
    renderData.instance = Renderer::g_instance;
    renderData.device = Renderer::g_device;
    renderData.renderPass = Renderer::g_renderPass;
    renderData.commandBuffer = Renderer::g_currentCommandBuffer;
    renderData.commandEncoder = Renderer::g_currentCommandEncoder;
    renderData.commandFramebuffer = Renderer::g_currentCommandFramebuffer;
    renderData.width = Renderer::g_width;
    renderData.height = Renderer::g_height;
    for (int i = 0; i < g_moduleRenders.size(); ++i)
    {
        PFN_MODULE_RENDER render = g_moduleRenders[i];
        render(renderData);
    }
}
//------------------------------------------------------------------------------
