//==============================================================================
// Minamoto : Setup Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <map>
#include <string>
#include <Runtime/Tools/CSV.h>
#include "Project.h"
#include "Setup.h"

static std::map<std::string, std::string> setups;
//------------------------------------------------------------------------------
void Setup::Initialize()
{
    Load();
}
//------------------------------------------------------------------------------
void Setup::Shutdown()
{
}
//------------------------------------------------------------------------------
void Setup::Load()
{
    std::string name = std::string(xxGetDocumentPath()) + "/.minamoto";
    CSV::Load(name.c_str(), [](std::vector<std::string_view> const& rows)
    {
        if (rows.size() < 2)
            return;
        if (rows[0].size() > 32)
            return;
        if (rows[0] == "ProjectPath")
        {
            Project::Root = rows[1];
            setups[std::string(rows[0])] = rows[1];
        }
    });
}
//------------------------------------------------------------------------------
void Setup::Save()
{
    setups["ProjectPath"] = Project::Root;

    auto it = setups.begin();
    std::string name = std::string(xxGetDocumentPath()) + "/.minamoto";
    CSV::Save(name.c_str(), [&](std::vector<std::string>& rows)
    {
        if (it == setups.end())
            return;
        auto const& pair = (*it++);
        rows.push_back(pair.first);
        rows.push_back(pair.second);
    });
}
//------------------------------------------------------------------------------
bool Setup::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    if (ImGui::Begin(ICON_FA_COG "Setup", &show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
    {
        for (auto& [key, value] : setups)
        {
            ImGui::SetNextItemWidth(256.0f);
            ImGui::InputText(key.c_str(), value.data(), value.size(), ImGuiInputTextFlags_ReadOnly);
        }
        if (ImGui::Button("Close"))
        {
            show = false;
        }
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
