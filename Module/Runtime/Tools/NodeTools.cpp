//==============================================================================
// Minamoto : NodeTools Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "NodeTools.h"

//==============================================================================
xxNodePtr const& NodeTools::GetRoot(xxNodePtr const& node)
{
    if (node == nullptr)
    {
        static xxNodePtr empty;
        return empty;
    }
    xxNodePtr const* trunk = &node;
    xxNodePtr const* root = nullptr;
    while ((root = &(*trunk)->GetParent()) && root->get())
    {
        trunk = root;
    }
    return (*trunk);
}
//------------------------------------------------------------------------------
xxNodePtr const& NodeTools::GetObject(xxNodePtr const& node, std::string const& name)
{
    xxNodePtr const* output = nullptr;
    xxNode::Traversal(node, [&](xxNodePtr const& node)
    {
        if (node->Name == name)
            output = &node;
        return output == nullptr;
    });
    if (output == nullptr)
    {
        static xxNodePtr empty;
        return empty;
    }
    return (*output);
}
//------------------------------------------------------------------------------
void NodeTools::UpdateNodeFlags(xxNodePtr const& node)
{
    xxNode::Traversal(node, [](xxNodePtr const& node)
    {
        node->Flags |= xxNode::UPDATE_SKIP;
        for (auto const& data : node->Bones)
        {
            if (data.bone.use_count())
            {
                xxNodePtr const& bone = (xxNodePtr&)data.bone;
                bone->Flags |= xxNode::UPDATE_NEED;
            }
        }
        if (node->Mesh)
        {
            node->Flags |= xxNode::UPDATE_NEED;
        }
        return true;
    });

    xxNode::Traversal(node, [](xxNodePtr const& node)
    {
        if (node->Flags & xxNode::UPDATE_NEED)
        {
            xxNodePtr parent = node;
            while (parent && (parent->Flags & xxNode::UPDATE_SKIP))
            {
                parent->Flags &= ~xxNode::UPDATE_SKIP;
                parent = parent->GetParent();
            }
        }
        return true;
    });
}
//==============================================================================
