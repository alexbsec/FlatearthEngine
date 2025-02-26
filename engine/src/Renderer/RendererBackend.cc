#include "RendererBackend.hpp"
#include "Core/FeMemory.hpp"
#include "Vulkan/VulkanBackend.hpp"

namespace flatearth {
namespace renderer {

bool CreateBackend(
    RendererBackendType type, struct platform::PlatformState *platState,
    core::memory::unique_renderer_ptr<RendererBackend> &outRenderer) {
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

void DestroyBackend(
    core::memory::unique_renderer_ptr<RendererBackend> &renderer) {
  renderer->Initialize = nullptr;
  renderer->Shutdown = nullptr;
  renderer->OnResize = nullptr;
  renderer->BeginFrame = nullptr;
  renderer->EndFrame = nullptr;
}

} // namespace renderer
} // namespace flatearth
