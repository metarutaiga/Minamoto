//==============================================================================
// Minamoto : ImportWavefront Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <xxGraphicPlus/xxFile.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxTexture.h>
#include "MeshTools.h"
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
static std::string CheckName(xxNodePtr const& node, std::string const& name)
{
    std::string output = name + "#0";
    for (size_t i = node->GetChildCount(); i > 0; --i)
    {
        xxNodePtr const& child = node->GetChild(i);
        if (child == nullptr)
            continue;
        if (child->Name.find(name) != 0)
            continue;
        output += '#';
        size_t sharp = child->Name.rfind('#');
        if (sharp == std::string::npos)
        {
            output += '0';
        }
        else
        {
            output += std::to_string(std::stoi(child->Name.substr(sharp + 1)) + 1);
        }
        break;
    }
    return output;
}
//------------------------------------------------------------------------------
static std::string GeneratePath(char const* path, char const* name)
{
    std::string output = xxFile::GetPath(path) + '/' + (name ? name : "");
    for (char& c : output)
    {
        if (c == '\\')
            c = '/';
    }
    return output;
}
//------------------------------------------------------------------------------
static xxTexturePtr CreateTexture(char const* img)
{
    xxTexturePtr texture = Import::CreateTexture(img);
    if (texture)
    {
        texture->ClampU = false;
        texture->ClampV = false;
        texture->ClampW = false;
    }
    return texture;
}
//------------------------------------------------------------------------------
std::map<std::string, ImportWavefront::Material> ImportWavefront::CreateMaterial(char const* mtl)
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
                if (material.output->SpecularColor == xxVector3::BLACK ||
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
            material.output->AlphaTest = material.output->Opacity == 1.0f;
            material.output->Blending = material.output->Opacity != 1.0f;
            if (material.map_Kd)
            {
                material.output->SetTexture(0, material.map_Kd);
            }
            materials[material.output->Name] = material;
        }
        material = {};
    };

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts;
        char const* delim = " \t";
        char const* statement = strtok_r(line, delim, &lasts);
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
        case xxHash("d"):
            material.output->Opacity = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Tr"):
            material.output->Opacity = 1.0f - ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Ka"):
            material.output->AmbientColor.r = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->AmbientColor.g = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->AmbientColor.b = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Kd"):
            material.output->DiffuseColor.r = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->DiffuseColor.g = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->DiffuseColor.b = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Ks"):
            material.output->SpecularColor.r = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->SpecularColor.g = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->SpecularColor.b = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Ke"):
            material.output->EmissiveColor.r = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->EmissiveColor.g = ToFloat(strtok_r(nullptr, delim, &lasts));
            material.output->EmissiveColor.b = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("Ns"):
            material.output->SpecularHighlight = ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("illum"):
            switch (ToInt(strtok_r(nullptr, delim, &lasts)))
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
            material.map_Ka = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Kd"):
            material.map_Kd = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Ks"):
            material.map_Ks = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Ns"):
            material.map_Ns = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_d"):
            material.map_d = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("decal"):
            material.decal = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("disp"):
            material.disp = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_bump"):
        case xxHash("bump"):
            material.bump = ::CreateTexture(GeneratePath(mtl, lasts).c_str());
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
    std::string name;
    Material* material = nullptr;
    xxNodePtr root;

    FILE* file = fopen(obj, "rb");
    if (file == nullptr)
        return root;

    root = xxNode::Create();
    root->Name = xxFile::GetName(obj);

    auto finish = [&]()
    {
        if (faceVertices.empty())
            return;
        xxNodePtr child = xxNode::Create();
        child->Name = CheckName(root, name);
        child->Mesh = MeshTools::CreateMesh(faceVertices, faceNormals, {}, faceTextures);
        if (child->Mesh)
        {
            child->Mesh->Name = child->Name;
        }
        if (material)
        {
            child->Material = material->output;
        }
        faceVertices.clear();
        faceNormals.clear();
        faceTextures.clear();
        root->AttachChild(child);
        material = nullptr;
    };

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts;
        char const* delim = " \t";
        char const* statement = strtok_r(line, delim, &lasts);
        if (statement == nullptr || lasts == nullptr)
            continue;

        switch (xxHash(statement))
        {
        case xxHash("mtllib"):
            materials = CreateMaterial(GeneratePath(obj, lasts).c_str());
            break;
        case xxHash("v"):
            finish();

            vertices.push_back(xxVector3::ZERO);
            vertices.back().x = ToFloat(strtok_r(nullptr, delim, &lasts));
            vertices.back().z = ToFloat(strtok_r(nullptr, delim, &lasts));
            vertices.back().y = -ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("vn"):
            normals.push_back(xxVector3::ZERO);
            normals.back().x = ToFloat(strtok_r(nullptr, delim, &lasts));
            normals.back().z = ToFloat(strtok_r(nullptr, delim, &lasts));
            normals.back().y = -ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("vt"):
            textures.push_back(xxVector2::ZERO);
            textures.back().x = 0 + ToFloat(strtok_r(nullptr, delim, &lasts));
            textures.back().y = 1 - ToFloat(strtok_r(nullptr, delim, &lasts));
            break;
        case xxHash("g"):
            finish();

            xxLog("ImportWavefront", "Create Node : %s", lasts);
            name = lasts;
            break;
        case xxHash("usemtl"):
            finish();

            xxLog("ImportWavefront", "Use Material : %s", lasts);
            material = &materials[lasts];
            break;
        case xxHash("f"):
            for (int i = 0; i < 16; ++i)
            {
                char* face = strtok_r(nullptr, delim, &lasts);
                if (face == nullptr)
                    break;

                // Triangle Fan
                if (i >= 3)
                {
                    size_t count;
                    count = faceVertices.size();
                    if (count >= i)
                    {
                        faceVertices.push_back(faceVertices[count - i]);
                        faceVertices.push_back(faceVertices[count - 1]);
                    }
                    count = faceNormals.size();
                    if (count >= i)
                    {
                        faceNormals.push_back(faceNormals[count - i]);
                        faceNormals.push_back(faceNormals[count - 1]);
                    }
                    count = faceTextures.size();
                    if (count >= i)
                    {
                        faceTextures.push_back(faceTextures[count - i]);
                        faceTextures.push_back(faceTextures[count - 1]);
                    }
                }

                char* parts = face;
                unsigned int v = ToInt(strsep(&parts, "/")) - 1;
                unsigned int t = ToInt(strsep(&parts, "/")) - 1;
                unsigned int n = ToInt(strsep(&parts, "/")) - 1;
                if (v < vertices.size())
                {
                    faceVertices.push_back(vertices[v]);
                }
                else if (vertices.empty() == false)
                {
                    xxLog("ImportWavefront", "Unknown vertice : %d", v);
                }
                if (n < normals.size())
                {
                    faceNormals.push_back(normals[n]);
                }
                else if (normals.empty() == false)
                {
                    xxLog("ImportWavefront", "Unknown normal : %d", n);
                }
                if (t < textures.size())
                {
                    faceTextures.push_back(textures[t]);
                }
                else if (textures.empty() == false)
                {
                    xxLog("ImportWavefront", "Unknown texture : %d", t);
                }
            }
            break;
        default:
            break;
        }
    }
    fclose(file);

    finish();

    if (root->GetChildCount() == 0)
    {
        return root->Mesh ? root : nullptr;
    }
    return root;
}
//==============================================================================
