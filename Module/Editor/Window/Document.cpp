//==============================================================================
// Minamoto : Document Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <functional>
#include <string>
#include <vector>
#include <xxGraphicPlus/xxFile.h>
#include "Document.h"

static std::vector<Document> documents;
//------------------------------------------------------------------------------
void Document::Initialize()
{
    
}
//------------------------------------------------------------------------------
void Document::Shutdown()
{
    documents = std::vector<Document>();
}
//------------------------------------------------------------------------------
bool Document::Update(const UpdateData& updateData)
{
    bool update = false;
    for (auto it = documents.begin(); it != documents.end(); ++it)
    {
        bool show = true;
        auto& document = (*it);
        ImGui::SetNextWindowSize(ImVec2(512.0f, 256.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(document.Title.c_str(), &show, ImGuiWindowFlags_NoDocking))
        {
            ImVec2 size = ImGui::GetContentRegionAvail();
            size.y -= ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;

            size_t length = document.Text.size();
            document.Text.resize(length + 1024);
            if (ImGui::InputTextMultiline("", document.Text.data(), document.Text.size(), size))
            {
                length = strlen(document.Text.data());
                update = true;
            }
            document.Text.resize(length);

            if (document.File.empty() == false)
            {
                if (ImGui::Button("Load"))
                {
                    xxFile* file = xxFile::Load(document.File.c_str());
                    if (file)
                    {
                        document.Text.resize(file->Size());
                        file->Read(document.Text.data(), document.Text.size());
                        delete file;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Save"))
                {
                    xxFile* file = xxFile::Save(document.File.c_str());
                    if (file)
                    {
                        file->Write(document.Text.data(), document.Text.size());
                        delete file;
                    }
                }
                ImGui::SameLine();
            }
            if (document.Callback)
            {
                document.Callback(updateData, document.Text);
            }
        }
        ImGui::End();
        if (show == false)
        {
            auto erase = it--;
            documents.erase(erase);
        }
    }
    return update;
}
//------------------------------------------------------------------------------
bool Document::OpenFile(char const* name, std::function<bool(UpdateData const&, std::string const&)> callback)
{
    for (auto& document : documents)
        if (document.File == name)
            return false;

    xxFile* file = xxFile::Load(name);
    if (file == nullptr)
        return false;

    size_t length = file->Size();
    std::string text(length, 0);
    file->Read(text.data(), length);

    Document document =
    {
        .Callback = callback,
        .File = name,
        .Text = std::move(text),
        .Title = name,
    };
    documents.push_back(document);
    return true;
}
//------------------------------------------------------------------------------
