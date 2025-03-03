#include "RendererBackend.hpp"
#include "Core/FeMemory.hpp"
#include "Renderer/RendererTypes.inl"
#include "Vulkan/VulkanBackend.hpp"
#include "Vulkan/VulkanBackendAbstract.hpp"
#include <unordered_map>

namespace flatearth {
namespace renderer {

void CreateBackend2(
    core::memory::unique_stateful_renderer_ptr<IRendererBackend> *backends,
    struct platform::PlatformState *platState) {
  void *allocatedMemory = nullptr;
  IRendererBackend *instance = nullptr;

  // Create Vulkan Backend
  allocatedMemory = core::memory::MemoryManager::Allocate(
      sizeof(vulkan::VulkanBackend), core::memory::MEMORY_TAG_RENDERER);
  if (allocatedMemory) {
    instance = new (allocatedMemory) vulkan::VulkanBackend();
    backends[RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN] =
        core::memory::unique_stateful_renderer_ptr<IRendererBackend>(
            instance, core::memory::StatefulCustomDeleter<IRendererBackend>(
                          sizeof(vulkan::VulkanBackend),
                          core::memory::MEMORY_TAG_RENDERER));
  }
}

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
