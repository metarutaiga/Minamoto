//==============================================================================
// Minamoto : ImportPLY Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <xxGraphicPlus/xxFile.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxNode.h>
#include "MeshTools.h"
#include "ImportPLY.h"

//==============================================================================
static void RemoveBreakline(char* text)
{
    if (char* slash = strrchr(text, '\r'))
        slash[0] = 0;
    if (char* slash = strrchr(text, '\n'))
        slash[0] = 0;
}
//------------------------------------------------------------------------------
static int ToInt(char const* text)
{
    return text ? atoi(text) : -1;
}
//------------------------------------------------------------------------------
static float ToFloat(char const* text)
{
    return text ? float(atof(text)) : 0.0f;
}
//------------------------------------------------------------------------------
xxNodePtr ImportPLY::Create(char const* ply)
{
    std::vector<char> attributes;
    std::vector<xxVector3> vertices;
    std::vector<uint16_t> faces16;
    std::vector<uint32_t> faces32;
    xxNodePtr root;

    FILE* file = fopen(ply, "rb");
    if (file == nullptr)
        return root;

    auto finish = [&]()
    {
        if (vertices.empty() == false)
        {
            if (root == nullptr)
            {
                root = xxNode::Create();
                root->Name = xxFile::GetName(ply);
                root->Material = xxMaterial::Create();
                root->Material->Lighting = true;
                root->Material->DiffuseColor = xxVector3::WHITE;
                root->Material->DepthTest = "LessEqual";
                root->Material->DepthWrite = true;
                root->Material->Cull = true;
                root->Material->Scissor = false;
            }
            xxMeshPtr mesh = root->Mesh = xxMesh::Create(false, 0, 0, 0);
            if (mesh)
            {
                mesh->Name = root->Name;
                if (vertices.size())
                {
                    mesh->SetVertexCount(static_cast<int>(vertices.size()));
                    auto source = vertices.begin();
                    for (auto& position : mesh->GetPosition())
                    {
                        position = (*source++);
                    }
                }
                if (faces16.size())
                {
                    mesh->SetIndexCount(static_cast<int>(faces16.size()));
                    memcpy(mesh->Index, faces16.data(), sizeof(uint16_t) * faces16.size());
                }
                else if (faces32.size())
                {
                    mesh->SetIndexCount(static_cast<int>(faces32.size()));
                    memcpy(mesh->Index, faces32.data(), sizeof(uint32_t) * faces32.size());
                }
                mesh = root->Mesh = MeshTools::OptimizeMesh(mesh);
                mesh = root->Mesh = MeshTools::CreateMeshlet(mesh);
                mesh = root->Mesh = MeshTools::NormalizeMesh(mesh);
                mesh->CalculateBound();
            }
            vertices.clear();
        }
    };

    size_t faceCount = 0;
    size_t vertexCount = 0;

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts = line;
        char const* statement = strsep(&lasts, " ");
        if (statement == nullptr)
            continue;

        switch (xxHash(statement))
        {
        case xxHash("comment"):
        case xxHash("format"):
        case xxHash("ply"):
            break;
        case xxHash("element"):
        {
            char const* type = strsep(&lasts, " ");
            char const* count = strsep(&lasts, " ");
            if (type == nullptr)
                break;
            switch (xxHash(type))
            {
            case xxHash("face"):
                faceCount = ToInt(count);
                break;
            case xxHash("vertex"):
                vertexCount = ToInt(count);
                break;
            default:
                break;
            }
            break;
        }
        case xxHash("property"):
        {
            char const* type = strsep(&lasts, " ");
            char const* attribute = strsep(&lasts, " ");
            if (type == nullptr || attribute == nullptr)
                break;
            switch (xxHash(type))
            {
            case xxHash("float"):
                switch (xxHash(attribute))
                {
                case xxHash("x"):
                    attributes.push_back('x');
                    break;
                case xxHash("y"):
                    attributes.push_back('y');
                    break;
                case xxHash("z"):
                    attributes.push_back('z');
                    break;
                default:
                    attributes.push_back('?');
                    break;
                }
                break;
            case xxHash("list"):
                break;
            }
            break;
        }
        case xxHash("end_header"):
        {
            while (fgets(line, 256, file))
            {
                RemoveBreakline(line);

                char* lasts = line;
                if (vertices.size() < vertexCount)
                {
                    xxVector3 vertex;
                    vertex.x = ToFloat(strsep(&lasts, " ")) * -100.0f;
                    vertex.z = ToFloat(strsep(&lasts, " ")) * 100.0f;
                    vertex.y = ToFloat(strsep(&lasts, " ")) * -100.0f;
                    vertices.push_back(vertex);
                }
                else if (vertexCount < 65536 && faces16.size() < faceCount * 3)
                {
                    size_t count = ToInt(strsep(&lasts, " "));
                    size_t pos = faces16.size();
                    for (size_t i = 0; i < count; ++i)
                        faces16.insert(faces16.begin() + pos, ToInt(strsep(&lasts, " ")));
                }
                else if (faces32.size() < faceCount * 3)
                {
                    size_t count = ToInt(strsep(&lasts, " "));
                    size_t pos = faces32.size();
                    for (size_t i = 0; i < count; ++i)
                        faces32.insert(faces32.begin() + pos, ToInt(strsep(&lasts, " ")));
                }
            }
            break;
        }
        default:
            break;
        }
    }
    fclose(file);

    finish();

    return root;
}
//==============================================================================
