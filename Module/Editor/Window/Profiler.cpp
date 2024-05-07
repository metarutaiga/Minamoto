//==============================================================================
// Minamoto : Profiler Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <map>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include "Profiler.h"

static std::map<unsigned int, std::pair<const char*, double>> times;
static std::map<unsigned int, std::pair<const char*, size_t>> counters;
//------------------------------------------------------------------------------
void Profiler::Initialize()
{
    
}
//------------------------------------------------------------------------------
void Profiler::Shutdown()
{
    times.clear();
    counters.clear();
}
//------------------------------------------------------------------------------
bool Profiler::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    if (ImGui::Begin(ICON_FA_BAR_CHART "Profiler", &show))
    {
        for (auto const& [hashName, pair] : times)
        {
            static uint64_t const min = 0;
            static uint64_t const max = NSEC_PER_SEC / 60;
            uint64_t value = uint64_t(pair.second * NSEC_PER_SEC);
            ImGui::SliderScalar(pair.first, ImGuiDataType_U64, &value, &min, &max, "%lluus", ImGuiInputTextFlags_ReadOnly);
        }
        for (auto const& [hashName, pair] : counters)
        {
            uint64_t value = uint64_t(pair.second);
            ImGui::InputScalar(pair.first, ImGuiDataType_U64, &value, nullptr, nullptr, "%llu", ImGuiInputTextFlags_ReadOnly);
        }
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
void Profiler::Begin(unsigned int hashName)
{
    double* time = nullptr;
    switch (hashName)
    {
    case xxHash("Scene Update"):
        time = &(times[hashName] = {"Scene Update", 0.0}).second;
        break;
    case xxHash("Scene Render"):
        time = &(times[hashName] = {"Scene Render", 0.0}).second;
        break;
    }
    if (time)
    {
        double fp64 = 0.0;
        xxGetCurrentTime(&fp64);
        (*time) = -fp64;
    }
}
//------------------------------------------------------------------------------
void Profiler::End(unsigned int hashName)
{
    double fp64 = 0.0;
    xxGetCurrentTime(&fp64);
    switch (hashName)
    {
    case xxHash("Scene Update"):
    case xxHash("Scene Render"):
        times[hashName].second += fp64;
        break;
    }
}
//------------------------------------------------------------------------------
void Profiler::Count(unsigned int hashName, size_t count)
{
    switch (hashName)
    {
    case xxHash("Bone Count"):
        counters[hashName] = {"Bone Count", count};
        break;
    case xxHash("Modifier Total Count"):
        counters[hashName] = {"Modifier Total Count", count};
        break;
    case xxHash("Modifier Active Count"):
        counters[hashName] = {"Modifier Active Count", count};
        break;
    case xxHash("Node Total Count"):
        counters[hashName] = {"Node Total Count", count};
        break;
    case xxHash("Node Active Count"):
        counters[hashName] = {"Node Active Count", count};
        break;
    }
}
//------------------------------------------------------------------------------
