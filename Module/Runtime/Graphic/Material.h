//==============================================================================
// Minamoto : Material Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"
#include <xxGraphicPlus/xxMaterial.h>

struct RuntimeAPI Material : public xxMaterial
{
public:
    enum TextureType
    {
        BASE            = 0,
        BUMP            = 1,
    };

public:
    void                Invalidate() override;
    void                Draw(xxDrawData const& data) const override;

    void                CreatePipeline(xxDrawData const& data) override;
    void                CreateConstant(xxDrawData const& data) const override;
    void                UpdateConstant(xxDrawData const& data) const override;

protected:
    std::string         GetShader(xxDrawData const& data, int type) const override;
    int                 GetMeshConstantSize(xxDrawData const& data) const;
    int                 GetVertexConstantSize(xxDrawData const& data) const override;
    int                 GetFragmentConstantSize(xxDrawData const& data) const override;

    void                UpdateAlphaTestingConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;
    void                UpdateBlendingConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;
    void                UpdateCullingConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;
    void                UpdateLightingConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;
    void                UpdateSkinningConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;
    void                UpdateWorldViewProjectionConstant(xxDrawData const& data, int& size, xxVector4** pointer = nullptr) const;

    uint64_t            m_meshShader = 0;
    uint16_t            m_meshTextureSlot = 0;
    uint16_t            m_vertexTextureSlot = 0;
    uint16_t            m_fragmentTextureSlot = 0;

public:
    bool                BackfaceCulling = false;
    bool                FrustumCulling = false;

    static void         Initialize();
    static void         Shutdown();

    static char const   DefaultShader[];
};
