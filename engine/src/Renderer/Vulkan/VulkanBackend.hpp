#ifndef _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
#define _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP

#include "Core/FeMemory.hpp"
#include "Platform/Platform.hpp"
#include "Renderer/RendererTypes.inl"

namespace flatearth {
namespace renderer {
namespace vulkan {

bool InitializeVulkan(
    core::memory::unique_renderer_ptr<RendererBackend> &backend,
    const char *applicationName, struct platform::PlatformState *platState);

void ShutdownVulkan(
    core::memory::unique_renderer_ptr<RendererBackend> &backend);

void OnResizeVulkan(core::memory::unique_renderer_ptr<RendererBackend> &backend,
                    ushort width, ushort height);

bool BeginFrameVulkan(
    core::memory::unique_renderer_ptr<RendererBackend> &backend,
    float32 deltaTime);

bool EndFrameVulkan(core::memory::unique_renderer_ptr<RendererBackend> &backend,
                    float32 deltaTime);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
