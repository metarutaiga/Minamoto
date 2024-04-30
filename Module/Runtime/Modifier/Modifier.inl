//==============================================================================
// Minamoto : Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

template<class T>
bool Modifier::UpdateKeyFactor(xxModifierData* data, float time, T*& A, T*& B, float& F)
{
    if (data->time == time)
        return false;
    data->time = time;

    auto key = (T*)Data.data();
    auto count = Data.size() / sizeof(T);

    if (count == 0)
        return false;
    if (count == 1)
    {
        A = &key[0];
        B = &key[0];
        F = 1.0f;
        return true;
    }
    if (count == 2)
    {
        A = &key[0];
        B = &key[1];
    }
    else
    {
        float duration = key[count - 1].time;
        size_t maximum = count - 2;
        size_t index = std::min(data->index, maximum);
        A = &key[index + 0];
        B = &key[index + 1];

        time = std::fmodf(time, duration);
        if (A->time > time)
        {
            data->index = index = 0;
            A = &key[0];
            B = &key[1];
            if (A->time > time)
            {
                return false;
            }
        }
        while (B->time < time && index < maximum)
        {
            data->index = ++index;
            ++A;
            ++B;
        }
    }

    float XY = B->time - A->time;
    F = (time - A->time) / XY;
    return true;
}

template<class T, class D>
bool Modifier::UpdateBakedFactor(xxModifierData* data, float time, D* baked, T*& A, T*& B, float& F)
{
    if (data->time == time)
        return false;
    data->time = time;

    time = std::fmodf(time, baked->duration);
    size_t index = data->index = size_t(time * baked->inverseFrequency);

    A = (T*)&baked->values[index];
    B = (T*)&baked->values[index + 1];
    F = time - index * baked->frequency;
    return true;
}

template<class T>
T Modifier::Lerp(T const& A, T const &B, float F)
{
    return A + (B - A) * F;
}
