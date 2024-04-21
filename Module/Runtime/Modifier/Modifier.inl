//==============================================================================
// Minamoto : Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

template<class T>
bool Modifier::GetUpdateRatio(xxModifierData* data, float time, T*& A, T*& B, float& X, float& Y)
{
    if (data->time == time)
        return false;
    auto key = (T*)Data.data();
    auto count = Data.size() / sizeof(T);

    if (count == 0)
        return false;
    if (count == 1)
    {
        A = &key[0];
        B = &key[0];
        X = 1.0f;
        Y = 0.0f;
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
    X = (time - A->time) / XY;
    Y = 1.0f - X;
    return true;
}
