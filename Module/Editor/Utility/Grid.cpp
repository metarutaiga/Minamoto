#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include "Grid.h"

//------------------------------------------------------------------------------
xxNodePtr Grid::Create(xxVector3 const& translate, xxVector2 const& size)
{
    xxMeshPtr mesh = xxMesh::Create(0, 1, 1);
    if (mesh == nullptr)
        return nullptr;

    int vertex_count = 4;
    mesh->SetVertexCount(vertex_count);
    xxStrideIterator<xxVector3> it_vertex = mesh->GetVertex();
    (*it_vertex++) = {-size.x,  size.y, 0};
    (*it_vertex++) = {-size.x, -size.y, 0};
    (*it_vertex++) = { size.x,  size.y, 0};
    (*it_vertex++) = { size.x, -size.y, 0};
    xxStrideIterator<uint32_t> it_color = mesh->GetColor(0);
    (*it_color++) = 0xFFFFFFFF;
    (*it_color++) = 0xFFFFFFFF;
    (*it_color++) = 0xFFFFFFFF;
    (*it_color++) = 0xFFFFFFFF;
    xxStrideIterator<xxVector2> it_texture = mesh->GetTexture(0);
    (*it_texture++) = {-size.x,  size.y};
    (*it_texture++) = {-size.x, -size.y};
    (*it_texture++) = { size.x,  size.y};
    (*it_texture++) = { size.x, -size.y};

    int index_count = 6;
    mesh->SetIndexCount(index_count);
    xxMesh::IndexType* it_index = mesh->GetIndex();
    (*it_index++) = 0;
    (*it_index++) = 1;
    (*it_index++) = 2;
    (*it_index++) = 3;
    (*it_index++) = 1;
    (*it_index++) = 2;

    xxMaterialPtr material = xxMaterial::Create();
    if (material == nullptr)
        return nullptr;
    material->Blending = true;
    material->DepthTest = "LessEqual";
    material->DepthWrite = false;

    xxImagePtr image = CreateTexture();
    if (image == nullptr)
        return nullptr;
    image->ClampU = false;
    image->ClampV = false;
    image->ClampW = false;
    image->FilterMag = true;
    image->FilterMin = true;
    image->FilterMip = true;
    image->Anisotropic = 16;

    xxNodePtr node = xxNode::Create();
    if (node == nullptr)
        return nullptr;
    node->Mesh = mesh;
    node->Material = material;
    node->SetImage(0, image);
    node->SetTranslate(translate);

    return node;
}
//------------------------------------------------------------------------------
xxImagePtr Grid::CreateTexture()
{
    int level = 8;

    xxImagePtr image = xxImage::Create2D(0, (1 << (level - 1)), (1 << (level - 1)), level);
    if (image == nullptr)
        return nullptr;

    for (int mipmap = 0; mipmap < level; ++mipmap)
    {
        int width = 1 << (level - mipmap - 1);
        int height = 1 << (level - mipmap - 1);
        uint32_t* pixel = (uint32_t*)(*image)(0, 0, 0, mipmap, 0);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                uint32_t color = 0;
                if (y == 0)
                    color = 0xFF000000;
                if (x == 0)
                    color = 0xFF000000;
                if (width == 4)
                    color &= 0xAA000000;
                if (width == 2)
                    color &= 0x55000000;
                if (width == 1)
                    color &= 0x00000000;
                (*pixel++) = color;
            }
        }
    }

    return image;
}
//------------------------------------------------------------------------------
