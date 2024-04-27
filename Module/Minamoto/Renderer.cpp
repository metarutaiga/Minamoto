//==============================================================================
// Minamoto : Renderer Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#include <xxGraphic/xxGraphicD3D5.h>
#include <xxGraphic/xxGraphicD3D6.h>
#include <xxGraphic/xxGraphicD3D7.h>
#include <xxGraphic/xxGraphicD3D8.h>
#include <xxGraphic/xxGraphicD3D8PS.h>
#include <xxGraphic/xxGraphicD3D9.h>
#include <xxGraphic/xxGraphicD3D9PS.h>
#include <xxGraphic/xxGraphicD3D9Ex.h>
#include <xxGraphic/xxGraphicD3D9On12.h>
#include <xxGraphic/xxGraphicD3D10.h>
#include <xxGraphic/xxGraphicD3D10_1.h>
#include <xxGraphic/xxGraphicD3D11.h>
#include <xxGraphic/xxGraphicD3D11On12.h>
#include <xxGraphic/xxGraphicD3D12.h>
#include <xxGraphic/xxGraphicGlide.h>
#include <xxGraphic/xxGraphicGLES2.h>
#include <xxGraphic/xxGraphicGLES3.h>
#include <xxGraphic/xxGraphicGLES31.h>
#include <xxGraphic/xxGraphicGLES32.h>
#include <xxGraphic/xxGraphicMetal.h>
#include <xxGraphic/xxGraphicMetal2.h>
#include <xxGraphic/xxGraphicNULL.h>
#include <xxGraphic/xxGraphicVulkan.h>

#include "Renderer.h"

uint64_t    Renderer::g_instance = 0;
uint64_t    Renderer::g_device = 0;
uint64_t    Renderer::g_renderPass = 0;
uint64_t    Renderer::g_swapchain = 0;
uint64_t    Renderer::g_currentCommandBuffer = 0;
uint64_t    Renderer::g_currentCommandEncoder = 0;
uint64_t    Renderer::g_currentCommandFramebuffer = 0;
void*       Renderer::g_view = nullptr;
int         Renderer::g_width = 0;
int         Renderer::g_height = 0;
float       Renderer::g_scale = 1.0f;
float       Renderer::g_clearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
float       Renderer::g_clearDepth = 1.0f;
char        Renderer::g_clearStencil = 0;
//==============================================================================
//  List
//==============================================================================
static const struct { const char* const shortName; const char* const fullName; uint64_t (*createInstance)(); } g_graphicList[] =
{
#if defined(xxWINDOWS)
#if defined(_M_IX86)
    { "D3D5",           xxGetInstanceNameD3D5(),            xxCreateInstanceD3D5            },
    { "D3D6",           xxGetInstanceNameD3D6(),            xxCreateInstanceD3D6            },
    { "D3D7",           xxGetInstanceNameD3D7(),            xxCreateInstanceD3D7            },
    { "D3D8",           xxGetInstanceNameD3D8(),            xxCreateInstanceD3D8            },
    { "D3D8PS",         xxGetInstanceNameD3D8PS(),          xxCreateInstanceD3D8PS          },
#endif
    { "D3D9",           xxGetInstanceNameD3D9(),            xxCreateInstanceD3D9            },
    { "D3D9PS",         xxGetInstanceNameD3D9PS(),          xxCreateInstanceD3D9PS          },
    { "D3D9Ex",         xxGetInstanceNameD3D9Ex(),          xxCreateInstanceD3D9Ex          },
    { "D3D9ExPS",       xxGetInstanceNameD3D9ExPS(),        xxCreateInstanceD3D9ExPS        },
    { "D3D9On12",       xxGetInstanceNameD3D9On12(),        xxCreateInstanceD3D9On12        },
    { "D3D9On12PS",     xxGetInstanceNameD3D9On12PS(),      xxCreateInstanceD3D9On12PS      },
    { "D3D9On12Ex",     xxGetInstanceNameD3D9On12Ex(),      xxCreateInstanceD3D9On12Ex      },
    { "D3D9On12ExPS",   xxGetInstanceNameD3D9On12ExPS(),    xxCreateInstanceD3D9On12ExPS    },
    { "D3D10",          xxGetInstanceNameD3D10(),           xxCreateInstanceD3D10           },
    { "D3D10_1",        xxGetInstanceNameD3D10_1(),         xxCreateInstanceD3D10_1         },
    { "D3D11",          xxGetInstanceNameD3D11(),           xxCreateInstanceD3D11           },
    { "D3D11On12",      xxGetInstanceNameD3D11On12(),       xxCreateInstanceD3D11On12       },
    { "D3D12",          xxGetInstanceNameD3D12(),           xxCreateInstanceD3D12           },
#endif
#if defined(xxMACOS) || defined(xxWINDOWS)
    { "Glide",          xxGetInstanceNameGlide(),            xxCreateInstanceGlide           },
#endif
#if defined(xxMACCATALYST)
#else
    { "GLES2",          xxGetInstanceNameGLES2(),           xxCreateInstanceGLES2           },
    { "GLES3",          xxGetInstanceNameGLES3(),           xxCreateInstanceGLES3           },
    { "GLES31",         xxGetInstanceNameGLES31(),          xxCreateInstanceGLES31          },
    { "GLES32",         xxGetInstanceNameGLES32(),          xxCreateInstanceGLES32          },
#endif
#if defined(xxMACOS) || defined(xxIOS)
    { "MTL",            xxGetInstanceNameMetal(),           xxCreateInstanceMetal           },
    { "MTL2",           xxGetInstanceNameMetal2(),          xxCreateInstanceMetal2          },
#endif
    { "NULL",           xxGetInstanceNameNULL(),            xxCreateInstanceNULL            },
    { "VK",             xxGetInstanceNameVulkan(),          xxCreateInstanceVulkan          },
};
//==============================================================================
//  Renderer
//==============================================================================
bool Renderer::Create(void* view, int width, int height, const char* shortName)
{
    if (g_instance != 0)
        return false;

    if (view == nullptr || width == 0 || height == 0)
        return false;

    if (shortName == nullptr)
    {
#if defined(xxWINDOWS)
        shortName = "D3D11";
#elif defined(xxMACOS) || defined(xxIOS)
        shortName = "MTL2";
#else
        shortName = "GLES2";
#endif
    }

    unsigned int hashShortName = xxHash(shortName);
    for (int i = 0; i < xxCountOf(g_graphicList); ++i)
    {
        if (xxHash(g_graphicList[i].shortName) != hashShortName)
            continue;
        g_instance = g_graphicList[i].createInstance();
        break;
    }
    if (g_instance == 0)
        return false;

    g_device = xxCreateDevice(g_instance);
    g_renderPass = xxCreateRenderPass(g_device, true, true, true, true, true, true);
    g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, width, height, 0);
    g_view = view;
    g_width = width;
    g_height = height;
    return true;
}
//------------------------------------------------------------------------------
void Renderer::Reset(void* view, int width, int height)
{
    if (g_swapchain == 0)
        return;

    if (view == nullptr || width == 0 || height == 0)
        return;

    uint64_t oldSwapchain = g_swapchain;
    g_swapchain = 0;
    g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, width, height, oldSwapchain);
    g_view = view;
    g_width = width;
    g_height = height;
}
//------------------------------------------------------------------------------
void Renderer::Shutdown()
{
    xxDestroySwapchain(g_swapchain);
    xxDestroyRenderPass(g_renderPass);
    xxDestroyDevice(g_device);
    xxDestroyInstance(g_instance);
    g_swapchain = 0;
    g_renderPass = 0;
    g_device = 0;
    g_instance = 0;
}
//------------------------------------------------------------------------------
uint64_t Renderer::Begin()
{
    uint64_t commandBuffer = xxGetCommandBuffer(g_device, g_swapchain);
    uint64_t framebuffer = xxGetFramebuffer(g_device, g_swapchain, &g_scale);
    xxBeginCommandBuffer(commandBuffer);

    int width = (int)(g_width * g_scale);
    int height = (int)(g_height * g_scale);
    uint64_t commandEncoder = xxBeginRenderPass(commandBuffer, framebuffer, g_renderPass, width, height, g_clearColor, g_clearDepth, g_clearStencil);
    xxSetViewport(commandEncoder, 0, 0, width, height, 0.0f, 1.0f);
    xxSetScissor(commandEncoder, 0, 0, width, height);

    g_currentCommandBuffer = commandBuffer;
    g_currentCommandEncoder = commandEncoder;
    g_currentCommandFramebuffer = framebuffer;

    return commandEncoder;
}
//------------------------------------------------------------------------------
void Renderer::End()
{
    xxEndRenderPass(g_currentCommandEncoder, g_currentCommandFramebuffer, g_renderPass);

    xxEndCommandBuffer(g_currentCommandBuffer);
    xxSubmitCommandBuffer(g_currentCommandBuffer, g_swapchain);

    g_currentCommandBuffer = 0;
    g_currentCommandEncoder = 0;
    g_currentCommandFramebuffer = 0;
}
//------------------------------------------------------------------------------
bool Renderer::Present()
{
    xxPresentSwapchain(g_swapchain);

    return xxTestDevice(g_device);
}
//------------------------------------------------------------------------------
const char* Renderer::GetCurrentFullName()
{
    return xxGetInstanceName();
}
//------------------------------------------------------------------------------
const char* Renderer::GetGraphicFullName(int index)
{
    if (index >= xxCountOf(g_graphicList))
        return nullptr;

    return g_graphicList[index].fullName;
}
//------------------------------------------------------------------------------
const char* Renderer::GetGraphicShortName(int index)
{
    if (index >= xxCountOf(g_graphicList))
        return nullptr;

    return g_graphicList[index].shortName;
}
//------------------------------------------------------------------------------
