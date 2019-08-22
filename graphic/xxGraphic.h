#pragma once

#include "xxSystem.h"

//==============================================================================
//  Instance
//==============================================================================
xxAPI uint64_t      (*xxCreateInstance)();
xxAPI void          (*xxDestroyInstance)(uint64_t instance);
//==============================================================================
//  Device
//==============================================================================
xxAPI uint64_t      (*xxCreateDevice)(uint64_t instance);
xxAPI void          (*xxDestroyDevice)(uint64_t device);
xxAPI void          (*xxResetDevice)(uint64_t device);
xxAPI bool          (*xxTestDevice)(uint64_t device);
xxAPI const char*   (*xxGetDeviceString)(uint64_t device);
//==============================================================================
//  Swapchain
//==============================================================================
xxAPI uint64_t      (*xxCreateSwapchain)(uint64_t device, void* view, unsigned int width, unsigned int height);
xxAPI void          (*xxDestroySwapchain)(uint64_t swapchain);
xxAPI void          (*xxPresentSwapchain)(uint64_t swapchain, void* view);
//==============================================================================
//  Command Buffer
//==============================================================================
xxAPI uint64_t      (*xxGetCommandBuffer)(uint64_t device, uint64_t swapchain);
xxAPI bool          (*xxBeginCommandBuffer)(uint64_t commandBuffer);
xxAPI void          (*xxEndCommandBuffer)(uint64_t commandBuffer);
xxAPI void          (*xxSubmitCommandBuffer)(uint64_t commandBuffer);
//==============================================================================
//  Render Pass
//==============================================================================
xxAPI uint64_t      (*xxCreateRenderPass)(uint64_t device, float r, float g, float b, float a, float depth, unsigned char stencil);
xxAPI void          (*xxDestroyRenderPass)(uint64_t renderPass);
xxAPI bool          (*xxBeginRenderPass)(uint64_t commandBuffer, uint64_t renderPass);
xxAPI void          (*xxEndRenderPass)(uint64_t commandBuffer, uint64_t renderPass);
//==============================================================================
//  Buffer
//==============================================================================
xxAPI uint64_t      (*xxCreateConstantBuffer)(uint64_t device, unsigned int size);
xxAPI uint64_t      (*xxCreateIndexBuffer)(uint64_t device, unsigned int size);
xxAPI uint64_t      (*xxCreateVertexBuffer)(uint64_t device, unsigned int size);
xxAPI void          (*xxDestroyBuffer)(uint64_t buffer);
xxAPI void*         (*xxMapBuffer)(uint64_t device, uint64_t buffer);
xxAPI void          (*xxUnmapBuffer)(uint64_t device, uint64_t buffer);
//==============================================================================
//  Texture
//==============================================================================
xxAPI uint64_t      (*xxCreateTexture)(uint64_t device, int format, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipmap, unsigned int array);
xxAPI void          (*xxDestroyTexture)(uint64_t texture);
xxAPI void*         (*xxMapTexture)(uint64_t device, uint64_t texture, unsigned int& stride, unsigned int level, unsigned int array, unsigned int mipmap);
xxAPI void          (*xxUnmapTexture)(uint64_t device, uint64_t texture, unsigned int level, unsigned int array, unsigned int mipmap);
//==============================================================================
//  Vertex Attribute
//==============================================================================
xxAPI uint64_t      (*xxCreateVertexAttribute)(uint64_t device, int count, ...);
xxAPI void          (*xxDestroyVertexAttribute)(uint64_t vertexAttribute);
//==============================================================================
//  Shader
//==============================================================================
xxAPI uint64_t      (*xxCreateVertexShader)(uint64_t device, const char* shader, uint64_t vertexAttribute);
xxAPI uint64_t      (*xxCreateFragmentShader)(uint64_t device, const char* shader);
xxAPI void          (*xxDestroyShader)(uint64_t device, uint64_t shader);
//==============================================================================
//  Pipeline
//==============================================================================
xxAPI uint64_t      (*xxCreateBlendState)(uint64_t device, bool blending);
xxAPI uint64_t      (*xxCreateDepthStencilState)(uint64_t device, bool depthTest, bool depthWrite);
xxAPI uint64_t      (*xxCreateRasterizerState)(uint64_t device, bool cull, bool scissor);
xxAPI uint64_t      (*xxCreatePipeline)(uint64_t device, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader);
xxAPI void          (*xxDestroyBlendState)(uint64_t blendState);
xxAPI void          (*xxDestroyDepthStencilState)(uint64_t depthStencilState);
xxAPI void          (*xxDestroyRasterizerState)(uint64_t rasterizerState);
xxAPI void          (*xxDestroyPipeline)(uint64_t pipeline);
//==============================================================================
//  Command
//==============================================================================
xxAPI void          (*xxSetViewport)(uint64_t commandBuffer, int x, int y, int width, int height, float minZ, float maxZ);
xxAPI void          (*xxSetScissor)(uint64_t commandBuffer, int x, int y, int width, int height);
xxAPI void          (*xxSetPipeline)(uint64_t commandBuffer, uint64_t pipeline);
xxAPI void          (*xxSetIndexBuffer)(uint64_t commandBuffer, uint64_t buffer);
xxAPI void          (*xxSetVertexBuffers)(uint64_t commandBuffer, int count, const uint64_t* buffers, uint64_t vertexAttribute);
xxAPI void          (*xxSetVertexTextures)(uint64_t commandBuffer, int count, const uint64_t* textures);
xxAPI void          (*xxSetFragmentTextures)(uint64_t commandBuffer, int count, const uint64_t* textures);
xxAPI void          (*xxSetVertexConstantBuffer)(uint64_t commandBuffer, uint64_t buffer, unsigned int size);
xxAPI void          (*xxSetFragmentConstantBuffer)(uint64_t commandBuffer, uint64_t buffer, unsigned int size);
xxAPI void          (*xxDrawIndexed)(uint64_t commandBuffer, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance);
//==============================================================================
//  Fixed-Function
//==============================================================================
xxAPI void          (*xxSetTransform)(uint64_t commandBuffer, const float* world, const float* view, const float* projection);