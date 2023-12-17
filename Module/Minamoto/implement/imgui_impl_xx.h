// dear imgui: Renderer for XX
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'uint64_t' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Renderer: Multi-viewport support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once

#include <xxGraphic/xxGraphic.h>

IMGUI_IMPL_API bool     ImGui_ImplXX_Init(uint64_t instance, uint64_t device, uint64_t renderPass);
IMGUI_IMPL_API void     ImGui_ImplXX_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplXX_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplXX_RenderDrawData(ImDrawData* draw_data, uint64_t commandEncoder);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_IMPL_API bool     ImGui_ImplXX_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplXX_InvalidateDeviceObjects();
