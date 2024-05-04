//==============================================================================
// Minamoto : About Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <freetype/freetype.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <lua/lua.h>
#include <ufbx/ufbx.h>
#include <Runtime/Runtime.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include "About.h"

#undef ufbx_pack_version
#define ufbx_pack_version(major, minor, patch) #major "." #minor "." #patch

//------------------------------------------------------------------------------
bool About::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    if (ImGui::Begin(ICON_FA_QUESTION_CIRCLE "About " MODULE_NAME, &show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
    {
        ImGui::Text("%s Module for %s", MODULE_NAME, Runtime::Version());
        ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
        ImGui::Separator();
        ImGui::DumpBuildInformation();
        ImGui::Separator();
        ImGui::TextUnformatted("FreeType : " xxStringify(FREETYPE_MAJOR) "." xxStringify(FREETYPE_MINOR) "." xxStringify(FREETYPE_PATCH));
        ImGui::TextUnformatted("Lua : " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE);
        ImGui::Separator();
        ImGui::TextUnformatted("ImGuiFileDialog : " IGFD_VERSION);
        ImGui::TextUnformatted("ufbx : " UFBX_VERSION);
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
