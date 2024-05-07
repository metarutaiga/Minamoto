//==============================================================================
// Minamoto : Sampler Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic.h>
#include "Sampler.h"

//==============================================================================
static uint64_t samplers[1 << 9];
//------------------------------------------------------------------------------
static uint64_t (*xxCreateSamplerSystem)(uint64_t device, bool clampU, bool clampV, bool clampW, bool linearMag, bool linearMin, bool linearMip, int anisotropy);
static void     (*xxDestroySamplerSystem)(uint64_t sampler);
//------------------------------------------------------------------------------
static uint64_t xxCreateSamplerRuntime(uint64_t device, bool clampU, bool clampV, bool clampW, bool linearMag, bool linearMin, bool linearMip, int anisotropy)
{
    auto log2 = [](int value)
    {
        return 31 - xxCountLeadingZeros(value);
    };

    uint16_t hash = 0;
    hash |= clampU              << 0;
    hash |= clampV              << 1;
    hash |= clampW              << 2;
    hash |= linearMag           << 3;
    hash |= linearMin           << 4;
    hash |= linearMip           << 5;
    hash |= log2(anisotropy)    << 6;

    uint64_t output = samplers[hash];
    if (output == 0)
    {
        output = samplers[hash] = xxCreateSamplerSystem(device, clampU, clampV, clampW, linearMag, linearMin, linearMip, anisotropy);
    }
    return output;
}
//------------------------------------------------------------------------------
static void xxDestroySamplerRuntime(uint64_t sampler)
{
}
//==============================================================================
void Sampler::Initialize()
{
    if (xxCreateSamplerSystem)
        return;
    xxCreateSamplerSystem = xxCreateSampler;
    xxDestroySamplerSystem = xxDestroySampler;
    xxCreateSampler = xxCreateSamplerRuntime;
    xxDestroySampler = xxDestroySamplerRuntime;
}
//------------------------------------------------------------------------------
void Sampler::Shutdown()
{
    if (xxCreateSamplerSystem == nullptr)
        return;
    for (auto& sampler : samplers)
    {
        if (sampler)
        {
            xxDestroySamplerSystem(sampler);
        }
        sampler = 0;
    }
    xxCreateSampler = xxCreateSamplerSystem;
    xxDestroySampler = xxDestroySamplerSystem;
    xxCreateSamplerSystem = nullptr;
    xxDestroySamplerSystem = nullptr;
}
//==============================================================================
