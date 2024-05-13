//==============================================================================
// Minamoto : ImportWavefront Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxFile.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include "ImportWavefront.h"

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
static std::string GeneratePath(char const* path, char const* name)
{
    return xxFile::GetPath(path) + (name ? name : "");
}
//------------------------------------------------------------------------------
std::map<std::string, ImportWavefront::Material> ImportWavefront::CreateMaterial(const char* mtl)
{
    std::map<std::string, Material> materials;
    Material material;

    FILE* file = fopen(mtl, "rb");
    if (file == nullptr)
        return materials;

    auto finish = [&]()
    {
        if (material.output)
        {
            if (material.output->Specular)
            {
                if (material.output->SpecularColor == xxVector3::BLACK &&
                    material.output->SpecularHighlight == 0.0f)
                {
                    material.output->Specular = false;
                }
            }
            if (material.output->Lighting)
            {
                if (material.output->AmbientColor == xxVector3::WHITE &&
                    material.output->DiffuseColor == xxVector3::BLACK &&
                    material.output->EmissiveColor == xxVector3::BLACK &&
                    material.output->Specular == false)
                {
                    material.output->Lighting = false;
                }
            }
            materials[material.output->Name] = material;
        }
        material = {};
    };

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts = line;
        const char* statement = strsep(&lasts, " ");
        if (statement == nullptr || lasts == nullptr)
            continue;

        if (xxHash(statement) == xxHash("newmtl"))
        {
            finish();

            xxLog("ImportWavefront", "Create Material : %s", lasts);
            material.output = xxMaterial::Create();
            material.output->Name = lasts;
            material.output->DepthTest = "LessEqual";
            material.output->DepthWrite = true;
            material.output->Cull = true;
        }
        if (material.output == nullptr)
            continue;

        switch (xxHash(statement))
        {
        case xxHash("Ka"):
            material.output->AmbientColor.r = ToFloat(strsep(&lasts, " "));
            material.output->AmbientColor.g = ToFloat(strsep(&lasts, " "));
            material.output->AmbientColor.b = ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("Kd"):
            material.output->DiffuseColor.r = ToFloat(strsep(&lasts, " "));
            material.output->DiffuseColor.g = ToFloat(strsep(&lasts, " "));
            material.output->DiffuseColor.b = ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("Ks"):
            material.output->SpecularColor.r = ToFloat(strsep(&lasts, " "));
            material.output->SpecularColor.g = ToFloat(strsep(&lasts, " "));
            material.output->SpecularColor.b = ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("Ke"):
            material.output->EmissiveColor.r = ToFloat(strsep(&lasts, " "));
            material.output->EmissiveColor.g = ToFloat(strsep(&lasts, " "));
            material.output->EmissiveColor.b = ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("Ns"):
            material.output->SpecularHighlight = ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("illum"):
            switch (ToInt(strsep(&lasts, " ")))
            {
            default:
            case 0:
                material.output->Lighting = false;
                material.output->Specular = false;
                break;
            case 1:
                material.output->Lighting = true;
                material.output->Specular = false;
                break;
            case 2:
                material.output->Lighting = true;
                material.output->Specular = true;
                break;
            }
            break;
        case xxHash("map_Ka"):
            material.map_Ka = CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Kd"):
            material.map_Kd = CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Ks"):
            material.map_Ks = CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        }
    }
    fclose(file);

    finish();

    return materials;
}
//------------------------------------------------------------------------------
xxNodePtr ImportWavefront::Create(char const* obj)
{
    std::map<std::string, Material> materials;
    std::vector<xxVector3> vertices;
    std::vector<xxVector3> normals;
    std::vector<xxVector2> textures;
    std::vector<xxVector3> faceVertices;
    std::vector<xxVector3> faceNormals;
    std::vector<xxVector2> faceTextures;
    xxNodePtr child;
    xxNodePtr root;

    FILE* file = fopen(obj, "rb");
    if (file == nullptr)
        return root;

    auto finish = [&]()
    {
        if (child == nullptr)
            return;
        if (faceVertices.empty() == false)
        {
            child->Mesh = CreateMesh(faceVertices, faceNormals, {}, faceTextures);
            if (child->Mesh)
            {
                child->Mesh->Name = child->Name;
            }
            faceVertices.clear();
            faceNormals.clear();
            faceTextures.clear();
        }
        if (root == nullptr)
        {
            root = xxNode::Create();
            root->Name = xxFile::GetName(obj);
        }
        root->AttachChild(child);
    };

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts = line;
        const char* statement = strsep(&lasts, " ");
        if (statement == nullptr || lasts == nullptr)
            continue;

        switch (xxHash(statement))
        {
        case xxHash("mtllib"):
            materials = CreateMaterial(GeneratePath(obj, lasts).c_str());
            break;
        case xxHash("v"):
            vertices.push_back(xxVector3::ZERO);
            vertices.back().x = ToFloat(strsep(&lasts, " "));
            vertices.back().z = ToFloat(strsep(&lasts, " "));
            vertices.back().y = -ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("vn"):
            normals.push_back(xxVector3::ZERO);
            normals.back().x = ToFloat(strsep(&lasts, " "));
            normals.back().z = ToFloat(strsep(&lasts, " "));
            normals.back().y = -ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("vt"):
            textures.push_back(xxVector2::ZERO);
            textures.back().x = 0 + ToFloat(strsep(&lasts, " "));
            textures.back().y = 1 - ToFloat(strsep(&lasts, " "));
            break;
        case xxHash("g"):
            finish();

            xxLog("ImportWavefront", "Create Node : %s", lasts);
            child = xxNode::Create();
            child->Name = lasts;
            break;
        case xxHash("usemtl"):
            if (child)
            {
                xxLog("ImportWavefront", "Use Material : %s", lasts);
                auto material = materials[lasts];
                child->Material = material.output;
                if (material.map_Kd)
                {
                    child->Material->SetTexture(0, material.map_Kd);
                }
            }
            break;
        case xxHash("f"):
            for (int i = 0; i < 3; ++i)
            {
                char* face = strsep(&lasts, " ");
                if (face == nullptr)
                    break;
                char* parts = face;
                unsigned int v = ToInt(strsep(&parts, "/")) - 1;
                unsigned int t = ToInt(strsep(&parts, "/")) - 1;
                unsigned int n = ToInt(strsep(&parts, "/")) - 1;
                if (v < vertices.size())
                {
                    faceVertices.push_back(vertices[v]);
                }
                if (n < normals.size())
                {
                    faceNormals.push_back(normals[n]);
                }
                if (t < textures.size())
                {
                    faceTextures.push_back(textures[t]);
                }
            }
            break;
        default:
            break;
        }
    }
    fclose(file);

    finish();

    return root;
}
//==============================================================================
