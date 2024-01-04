//==============================================================================
// Minamoto : ImportWavefront Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxMaterial.h>
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
    return text ? atof(text) : 0.0f;
}
//------------------------------------------------------------------------------
static std::string GeneratePath(char const* path, char const* name)
{
    size_t pos;
    std::string output = path;
    if ((pos = output.rfind('/')) != std::string::npos)
        output.resize(pos + 1);
    else if ((pos = output.rfind('\\')) != std::string::npos)
        output.resize(pos + 1);
    if (name)
        output += name;
    return output;
}
//------------------------------------------------------------------------------
static std::string GetName(char const* path)
{
    const char* leftName = strrchr(path, '/');
    if (leftName == nullptr)
        leftName = strrchr(path, '\\');

    if (leftName)
        leftName++;
    else if (leftName == nullptr)
        leftName = path;

    const char* rightName = strrchr(path, '.');
    if (rightName == nullptr)
        rightName = path + strlen(path);

    return std::string(leftName, rightName);
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

        char* lasts;
        const char* statement = strtok_r(line, " ", &lasts);
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
            material.output->AmbientColor.r = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->AmbientColor.g = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->AmbientColor.b = ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("Kd"):
            material.output->DiffuseColor.r = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->DiffuseColor.g = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->DiffuseColor.b = ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("Ks"):
            material.output->SpecularColor.r = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->SpecularColor.g = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->SpecularColor.b = ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("Ke"):
            material.output->EmissiveColor.r = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->EmissiveColor.g = ToFloat(strtok_r(nullptr, " ", &lasts));
            material.output->EmissiveColor.b = ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("Ns"):
            material.output->SpecularHighlight = ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("illum"):
            switch (ToInt(strtok_r(nullptr, " ", &lasts)))
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
            material.map_Ka = CreateImage(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Kd"):
            material.map_Kd = CreateImage(GeneratePath(mtl, lasts).c_str());
            break;
        case xxHash("map_Ks"):
            material.map_Ks = CreateImage(GeneratePath(mtl, lasts).c_str());
            break;
        }
    }
    fclose(file);

    finish();

    return materials;
}
//------------------------------------------------------------------------------
xxNodePtr ImportWavefront::CreateObject(char const* obj)
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
            faceVertices.clear();
            faceNormals.clear();
            faceTextures.clear();
        }
        if (root == nullptr)
        {
            root = xxNode::Create();
            root->Name = GetName(obj);
        }
        root->AttachChild(child);
    };

    char line[256];
    while (fgets(line, 256, file))
    {
        RemoveBreakline(line);

        char* lasts;
        const char* statement = strtok_r(line, " ", &lasts);
        if (statement == nullptr || lasts == nullptr)
            continue;

        switch (xxHash(statement))
        {
        case xxHash("mtllib"):
            materials = CreateMaterial(GeneratePath(obj, lasts).c_str());
            break;
        case xxHash("v"):
            vertices.push_back(xxVector3::ZERO);
            vertices.back().x = ToFloat(strtok_r(nullptr, " ", &lasts));
            vertices.back().z = ToFloat(strtok_r(nullptr, " ", &lasts));
            vertices.back().y = -ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("vn"):
            normals.push_back(xxVector3::ZERO);
            normals.back().x = ToFloat(strtok_r(nullptr, " ", &lasts));
            normals.back().z = ToFloat(strtok_r(nullptr, " ", &lasts));
            normals.back().y = -ToFloat(strtok_r(nullptr, " ", &lasts));
            break;
        case xxHash("vt"):
            textures.push_back(xxVector2::ZERO);
            textures.back().x = 0 + ToFloat(strtok_r(nullptr, " ", &lasts));
            textures.back().y = 1 - ToFloat(strtok_r(nullptr, " ", &lasts));
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
                    child->SetImage(0, material.map_Kd);
                }
            }
            break;
        case xxHash("f"):
            for (int i = 0; i < 3; ++i)
            {
                char* face = strtok_r(nullptr, " ", &lasts);
                if (face == nullptr)
                    break;
                char* parts;
                int v = ToInt(strtok_r(face, "/", &parts)) - 1;
                int t = ToInt(strtok_r(nullptr, "/", &parts)) - 1;
                int n = ToInt(strtok_r(nullptr, "/", &parts)) - 1;
                if (v >= 0 && v < vertices.size())
                {
                    faceVertices.push_back(vertices[v]);
                }
                if (n >= 0 && n < normals.size())
                {
                    faceNormals.push_back(normals[n]);
                }
                if (t >= 0 && t < textures.size())
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
