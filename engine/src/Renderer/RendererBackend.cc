#include "RendererBackend.hpp"
#include "Vulkan/VulkanBackend.hpp"

namespace flatearth {
namespace renderer {

bool CreateBackend(RendererBackendType type,
                   struct platform::PlatformState *platState,
                   unique_backend_renderer_ptr &outRenderer) {
  outRenderer->platState = platState;

  if (type == RENDERER_BACKEND_TYPE_VULKAN) {
    outRenderer->Initialize = vulkan::InitializeVulkan;
    outRenderer->Shutdown = vulkan::ShutdownVulkan;
    outRenderer->OnResize = vulkan::OnResizeVulkan;
    outRenderer->BeginFrame = vulkan::BeginFrameVulkan;
    outRenderer->EndFrame = vulkan::EndFrameVulkan;
    return FeTrue;
  }

  return FeFalse;
}

void DestroyBackend(unique_backend_renderer_ptr &renderer) {
  renderer->Initialize = nullptr;
  renderer->Shutdown = nullptr;
  renderer->OnResize = nullptr;
  renderer->BeginFrame = nullptr;
  renderer->EndFrame = nullptr;
}

} // namespace renderer
} // namespace flatearth
