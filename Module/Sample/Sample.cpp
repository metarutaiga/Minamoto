//==============================================================================
// Minamoto : Sample Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>

#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
#   include <sys/time.h>
#endif

#define MODULE_NAME     "Sample"
#define MODULE_MAJOR    1
#define MODULE_MINOR    0

//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    return MODULE_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
moduleAPI bool Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showClock = false;
    static bool showTimer = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(MODULE_NAME))
        {
            ImGui::MenuItem("About " MODULE_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Clock", nullptr, &showClock);
            ImGui::Separator();
            ImGui::MenuItem("Timer", nullptr, &showTimer);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " MODULE_NAME, &showAbout, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
        {
            ImGui::Text("%s Module Version %d.%d", MODULE_NAME, MODULE_MAJOR, MODULE_MINOR);
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::Separator();
            ImGui::DumpBuildInformation();
            ImGui::End();
        }
    }

    if (showClock)
    {
        if (ImGui::Begin("Clock", &showClock, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
        {
            static char textPrevious[64];
            static char textCurrent[64];
            time_t t = time(nullptr);
            struct tm* tm = localtime(&t);
            strftime(textCurrent, sizeof(textCurrent), "%c", tm);
            ImGui::TextUnformatted(textCurrent);
            if (strcmp(textPrevious, textCurrent) != 0)
            {
                strcpy(textPrevious, textCurrent);
                updated = true;
            }
        }
    }

    if (showTimer)
    {
        if (ImGui::Begin("Timer", &showTimer, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
        {
            ImGui::Text("Current Time : %f", ImGui::GetTime());
            ImGui::Text("Delta Time : %f", ImGuiIO().DeltaTime);
#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
            static timeval startSystem;
            static float startLibrary;
            timeval system;
            gettimeofday(&system, nullptr);
            float library = ImGui::GetTime() * 1000.0f;
            if (startSystem.tv_sec == 0)
                startSystem = system;
            if (startLibrary == 0.0f)
                startLibrary = library;
            float floatSystem = (system.tv_sec - startSystem.tv_sec) * 1000.0f + (system.tv_usec - startSystem.tv_usec) / 1000.0f;
            float accuracy = (library - startLibrary) - floatSystem;
            ImGui::Text("Accuracy Time : %f=%f-%f", accuracy, (library - startLibrary), floatSystem);
#endif
            updated = true;
        }
    }

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
