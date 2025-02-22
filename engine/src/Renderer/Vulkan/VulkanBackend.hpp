#ifndef _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
#define _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP

#include "Platform/Platform.hpp"
#include "Renderer/RendererTypes.inl"
#include "VulkanTypes.inl"

namespace flatearth {
namespace renderer {
namespace vulkan {

bool InitializeVulkan(unique_backend_renderer_ptr &backend,
                      const char *applicationName,
                      struct platform::PlatformState *platState);

void ShutdownVulkan(unique_backend_renderer_ptr &backend);

void OnResizeVulkan(unique_backend_renderer_ptr &backend, ushort width,
                    ushort height);

bool BeginFrameVulkan(unique_backend_renderer_ptr &backend, float32 deltaTime);

bool EndFrameVulkan(unique_backend_renderer_ptr &backend, float32 deltaTime);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
