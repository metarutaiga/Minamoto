#pragma once

#include "xxSystem.h"

xxGL_API uint64_t (*xxCreateInstance)();
xxGL_API void (*xxDestroyInstance)(uint64_t instance);

xxGL_API uint64_t (*xxCreateDevice)(uint64_t instance);
xxGL_API void (*xxDestroyDevice)(uint64_t device);
xxGL_API void (*xxResetDevice)(uint64_t device);
xxGL_API bool (*xxTestDevice)(uint64_t device);

xxGL_API uint64_t (*xxCreateSwapchain)(uint64_t device, void* view, unsigned int width, unsigned int height);
xxGL_API void (*xxDestroySwapchain)(uint64_t swapchain);
xxGL_API void (*xxPresentSwapchain)(uint64_t swapchain, void* view);

xxGL_API uint64_t (*xxGetCommandBuffer)(uint64_t device, uint64_t swapchain);
xxGL_API bool (*xxBeginCommandBuffer)(uint64_t commandBuffer);
xxGL_API void (*xxEndCommandBuffer)(uint64_t commandBuffer);
xxGL_API void (*xxSubmitCommandBuffer)(uint64_t commandBuffer);

xxGL_API uint64_t (*xxCreateRenderPass)(uint64_t device, float r, float g, float b, float a, float depth, unsigned char stencil);
xxGL_API void (*xxDestroyRenderPass)(uint64_t renderPass);
xxGL_API bool (*xxBeginRenderPass)(uint64_t commandBuffer, uint64_t renderPass);
xxGL_API void (*xxEndRenderPass)(uint64_t commandBuffer, uint64_t renderPass);

xxGL_API uint64_t (*xxCreateBuffer)(uint64_t device, unsigned int size, bool indexBuffer);
xxGL_API void (*xxDestroyBuffer)(uint64_t buffer);
xxGL_API void* (*xxMapBuffer)(uint64_t buffer);
xxGL_API void (*xxUnmapBuffer)(uint64_t buffer);

xxGL_API uint64_t (*xxCreateTexture)(uint64_t device, int format, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipmap, unsigned int array);
xxGL_API void (*xxDestroyTexture)(uint64_t texture);
xxGL_API void* (*xxMapTexture)(uint64_t texture, unsigned int& stride, unsigned int mipmap, unsigned int array);
xxGL_API void (*xxUnmapTexture)(uint64_t texture, unsigned int mipmap, unsigned int array);

xxGL_API uint64_t (*xxCreateVertexAttribute)(uint64_t device, int count, ...);
xxGL_API void (*xxDestroyVertexAttribute)(uint64_t vertexAttribute);

xxGL_API void (*xxSetViewport)(uint64_t commandBuffer, int x, int y, int width, int height, float minZ, float maxZ);
xxGL_API void (*xxSetScissor)(uint64_t commandBuffer, int x, int y, int width, int height);

xxGL_API void (*xxSetIndexBuffer)(uint64_t commandBuffer, uint64_t buffer);
xxGL_API void (*xxSetVertexBuffers)(uint64_t commandBuffer, const uint64_t* buffers, const int* offsets, const int* strides, int count);
xxGL_API void (*xxSetFragmentBuffers)(uint64_t commandBuffer, const uint64_t* buffers, const int* offsets, const int* strides, int count);

xxGL_API void (*xxSetVertexTextures)(uint64_t commandBuffer, const uint64_t* textures, int count);
xxGL_API void (*xxSetFragmentTextures)(uint64_t commandBuffer, const uint64_t* textures, int count);

xxGL_API void (*xxSetVertexAttribute)(uint64_t commandBuffer, uint64_t vertexAttribute);

xxGL_API void (*xxDrawIndexed)(uint64_t commandBuffer, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance);

// Fixed-Function
xxGL_API void (*xxSetOrthographicTransform)(uint64_t commandBuffer, float left, float right, float top, float bottom);

#if xxGraphicSource
#   define xxRegisterFunction(API) \
        xxCreateInstance = xxCreateInstance ## API; \
        xxDestroyInstance = xxDestroyInstance ## API; \
\
        xxCreateDevice = xxCreateDevice ## API; \
        xxDestroyDevice = xxDestroyDevice ## API; \
        xxResetDevice = xxResetDevice ## API; \
        xxTestDevice = xxTestDevice ## API; \
\
        xxCreateSwapchain = xxCreateSwapchain ## API; \
        xxDestroySwapchain = xxDestroySwapchain ## API; \
        xxPresentSwapchain = xxPresentSwapchain ## API; \
\
        xxGetCommandBuffer = xxGetCommandBuffer ## API; \
        xxBeginCommandBuffer = xxBeginCommandBuffer ## API; \
        xxEndCommandBuffer = xxEndCommandBuffer ## API; \
        xxSubmitCommandBuffer = xxSubmitCommandBuffer ## API; \
\
        xxCreateRenderPass = xxCreateRenderPass ## API; \
        xxDestroyRenderPass = xxDestroyRenderPass ## API; \
        xxBeginRenderPass = xxBeginRenderPass ## API; \
        xxEndRenderPass = xxEndRenderPass ## API; \
\
        xxCreateBuffer = xxCreateBuffer ## API; \
        xxDestroyBuffer = xxDestroyBuffer ## API; \
        xxMapBuffer = xxMapBuffer ## API; \
        xxUnmapBuffer = xxUnmapBuffer ## API; \
\
        xxCreateTexture = xxCreateTexture ## API; \
        xxDestroyTexture = xxDestroyTexture ## API; \
        xxMapTexture = xxMapTexture ## API; \
        xxUnmapTexture = xxUnmapTexture ## API; \
\
        xxCreateVertexAttribute = xxCreateVertexAttribute ## API; \
        xxDestroyVertexAttribute = xxDestroyVertexAttribute ## API; \
\
        xxSetViewport = xxSetViewport ## API; \
        xxSetScissor = xxSetScissor ## API; \
\
        xxSetIndexBuffer = xxSetIndexBuffer ## API; \
        xxSetVertexBuffers = xxSetVertexBuffers ## API; \
        xxSetFragmentBuffers = xxSetFragmentBuffers ## API; \
\
        xxSetVertexTextures = xxSetVertexTextures ## API; \
        xxSetFragmentTextures = xxSetFragmentTextures ## API; \
\
        xxSetVertexAttribute = xxSetVertexAttribute ## API; \
\
        xxDrawIndexed = xxDrawIndexed ## API; \
\
        xxSetOrthographicTransform = xxSetOrthographicTransform ## API;
#endif
