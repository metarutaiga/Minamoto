//==============================================================================
// Minamoto : Inspector Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class Inspector
{
    static float windowWidth;
    static xxNodePtr selected;
public:
    static void Initialize();
    static void Shutdown();
    static void Select(xxNodePtr const& node);
    static bool Update(const UpdateData& updateData, float menuBarHeight, bool& show, xxCameraPtr const& camera);
    static void UpdateCamera(const UpdateData& updateData, xxCameraPtr const& camera);
    static void UpdateNode(const UpdateData& updateData, xxNodePtr const& node);
    static void UpdateMaterial(const UpdateData& updateData, xxMaterialPtr const& material);
    static void UpdateMesh(const UpdateData& updateData, xxMeshPtr const& mesh);
    static void UpdateModifier(const UpdateData& updateData, std::vector<xxModifierData> const& modifierData);
};
