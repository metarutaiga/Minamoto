//==============================================================================
// xxImGui : Plugin Sample Source
//
// Copyright (c) 2019-2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>

#define PLUGIN_NAME     "Sample"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
pluginAPI bool Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showClock = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Clock", nullptr, &showClock);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " PLUGIN_NAME, &showAbout))
        {
            ImGui::Text("%s Plugin Version %d.%d", PLUGIN_NAME, PLUGIN_MAJOR, PLUGIN_MINOR);
            ImGui::Separator();
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::End();
        }
    }

    if (showClock)
    {
        if (ImGui::Begin("Clock", &showClock))
        {
            char text[64];
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(text, sizeof(text), "%c", tm);
            ImGui::TextUnformatted(text);
        }
    }

    return false;
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
