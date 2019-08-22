#include "xxGraphicNULL.h"
#include "xxGraphicInternal.h"

//==============================================================================
//  Instance
//==============================================================================
uint64_t xxCreateInstanceNULL()
{
    xxRegisterFunction(NULL);
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyInstanceNULL(uint64_t instance)
{
    xxUnregisterFunction();
}
//==============================================================================
//  Device
//==============================================================================
uint64_t xxCreateDeviceNULL(uint64_t instance)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyDeviceNULL(uint64_t device)
{

}
//------------------------------------------------------------------------------
void xxResetDeviceNULL(uint64_t device)
{

}
//------------------------------------------------------------------------------
bool xxTestDeviceNULL(uint64_t device)
{
    return true;
}
//------------------------------------------------------------------------------
const char* xxGetDeviceStringNULL(uint64_t device)
{
    return "NULL";
}
//==============================================================================
//  Swapchain
//==============================================================================
uint64_t xxCreateSwapchainNULL(uint64_t device, void* view, unsigned int width, unsigned int height)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroySwapchainNULL(uint64_t swapchain)
{

}
//------------------------------------------------------------------------------
void xxPresentSwapchainNULL(uint64_t swapchain, void* view)
{

}
//==============================================================================
//  Command Buffer
//==============================================================================
uint64_t xxGetCommandBufferNULL(uint64_t device, uint64_t swapchain)
{
    return 0;
}
//------------------------------------------------------------------------------
bool xxBeginCommandBufferNULL(uint64_t commandBuffer)
{
    return true;
}
//------------------------------------------------------------------------------
void xxEndCommandBufferNULL(uint64_t commandBuffer)
{

}
//------------------------------------------------------------------------------
void xxSubmitCommandBufferNULL(uint64_t commandBuffer)
{
}
//==============================================================================
//  Render Pass
//==============================================================================
uint64_t xxCreateRenderPassNULL(uint64_t device, float r, float g, float b, float a, float depth, unsigned char stencil)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyRenderPassNULL(uint64_t renderPass)
{

}
//------------------------------------------------------------------------------
bool xxBeginRenderPassNULL(uint64_t commandBuffer, uint64_t renderPass)
{
    return true;
}
//------------------------------------------------------------------------------
void xxEndRenderPassNULL(uint64_t commandBuffer, uint64_t renderPass)
{

}
//==============================================================================
//  Buffer
//==============================================================================
uint64_t xxCreateConstantBufferNULL(uint64_t device, unsigned int size)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateIndexBufferNULL(uint64_t device, unsigned int size)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateVertexBufferNULL(uint64_t device, unsigned int size)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyBufferNULL(uint64_t buffer)
{

}
//------------------------------------------------------------------------------
void* xxMapBufferNULL(uint64_t device, uint64_t buffer)
{
    return nullptr;
}
//------------------------------------------------------------------------------
void xxUnmapBufferNULL(uint64_t device, uint64_t buffer)
{

}
//==============================================================================
//  Texture
//==============================================================================
uint64_t xxCreateTextureNULL(uint64_t device, int format, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipmap, unsigned int array)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyTextureNULL(uint64_t texture)
{

}
//------------------------------------------------------------------------------
void* xxMapTextureNULL(uint64_t device, uint64_t texture, unsigned int& stride, unsigned int level, unsigned int array, unsigned int mipmap)
{
    return nullptr;
}
//------------------------------------------------------------------------------
void xxUnmapTextureNULL(uint64_t device, uint64_t texture, unsigned int level, unsigned int array, unsigned int mipmap)
{

}
//==============================================================================
//  Vertex Attribute
//==============================================================================
uint64_t xxCreateVertexAttributeNULL(uint64_t device, int count, ...)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyVertexAttributeNULL(uint64_t vertexAttribute)
{

}
//==============================================================================
//  Pipeline
//==============================================================================
uint64_t xxCreateBlendStateNULL(uint64_t device, bool blending)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateDepthStencilStateNULL(uint64_t device, bool depthTest, bool depthWrite)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateRasterizerStateNULL(uint64_t device, bool cull, bool scissor)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreatePipelineNULL(uint64_t device, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyBlendStateNULL(uint64_t blendState)
{

}
//------------------------------------------------------------------------------
void xxDestroyDepthStencilStateNULL(uint64_t depthStencilState)
{

}
//------------------------------------------------------------------------------
void xxDestroyRasterizerStateNULL(uint64_t rasterizerState)
{

}
//------------------------------------------------------------------------------
void xxDestroyPipelineNULL(uint64_t pipelineState)
{

}
//==============================================================================
//  Shader
//==============================================================================
uint64_t xxCreateVertexShaderNULL(uint64_t device, const char* shader, uint64_t vertexAttribute)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateFragmentShaderNULL(uint64_t device, const char* shader)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyShaderNULL(uint64_t device, uint64_t shader)
{

}
//==============================================================================
//  Command
//==============================================================================
void xxSetViewportNULL(uint64_t commandBuffer, int x, int y, int width, int height, float minZ, float maxZ)
{

}
//------------------------------------------------------------------------------
void xxSetScissorNULL(uint64_t commandBuffer, int x, int y, int width, int height)
{

}
//------------------------------------------------------------------------------
void xxSetPipelineNULL(uint64_t commandBuffer, uint64_t pipeline)
{

}
//------------------------------------------------------------------------------
void xxSetIndexBufferNULL(uint64_t commandBuffer, uint64_t buffer)
{

}
//------------------------------------------------------------------------------
void xxSetVertexBuffersNULL(uint64_t commandBuffer, int count, const uint64_t* buffers, uint64_t vertexAttribute)
{

}
//------------------------------------------------------------------------------
void xxSetVertexTexturesNULL(uint64_t commandBuffer, int count, const uint64_t* textures)
{

}
//------------------------------------------------------------------------------
void xxSetFragmentTexturesNULL(uint64_t commandBuffer, int count, const uint64_t* textures)
{

}
//------------------------------------------------------------------------------
void xxSetVertexConstantBufferNULL(uint64_t commandBuffer, uint64_t buffer, unsigned int size)
{

}
//------------------------------------------------------------------------------
void xxSetFragmentConstantBufferNULL(uint64_t commandBuffer, uint64_t buffer, unsigned int size)
{

}
//------------------------------------------------------------------------------
void xxDrawIndexedNULL(uint64_t commandBuffer, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance)
{

}
//==============================================================================
//  Fixed-Function
//==============================================================================
void xxSetTransformNULL(uint64_t commandBuffer, const float* world, const float* view, const float* projection)
{

}
//==============================================================================