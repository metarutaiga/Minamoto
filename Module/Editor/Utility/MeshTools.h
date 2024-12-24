//==============================================================================
// Minamoto : MeshTools Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct MeshTools
{
    struct MeshData
    {
        bool skinning = false;
        int normalCount = 0;
        int colorCount = 0;
        int textureCount = 0;
        int storageCount[6] = {};
        int storageStride[6] = {};

        std::vector<xxVector3> positions;
        std::vector<xxVector3> boneWeights;
        std::vector<uint32_t> boneIndices;
        std::vector<xxVector3> normals;
        std::vector<uint32_t> colors;
        std::vector<xxVector2> textures;
        std::vector<uint32_t> indices;
        std::vector<char> storages[6];
    };
    static std::vector<uint32_t> GetIndexFromMesh(xxMeshPtr const& mesh);
    static void SetIndexToMesh(xxMeshPtr const& mesh, std::vector<uint32_t> const& indices);
    static MeshData CreateMeshDataFromMesh(xxMeshPtr const& mesh);
    static xxMeshPtr CreateMeshFromMeshData(MeshData const& data);
    static xxMeshPtr CreateMesh(std::vector<xxVector3> const& vertices, std::vector<xxVector3> const& normals, std::vector<xxVector4> const& colors, std::vector<xxVector2> const& textures);
    static xxMeshPtr CreateMeshlet(xxMeshPtr const& mesh);
    static xxMeshPtr IndexingMesh(xxMeshPtr const& mesh);
    static xxMeshPtr NormalizeMesh(xxMeshPtr const& mesh);
    static xxMeshPtr OptimizeMesh(xxMeshPtr const& mesh);
};
