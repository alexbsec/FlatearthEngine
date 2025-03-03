#include "RendererFrontend.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Platform/Platform.hpp"
#include "Renderer/RendererTypes.inl"
#include "Vulkan/VulkanBackendAbstract.hpp"

#include <stdexcept>

namespace flatearth {
namespace renderer {

static core::memory::unique_stateful_renderer_ptr<IRendererBackend>
BackendCreate(const std::string &applicationName,
              struct platform::PlatformState *platState) {
  // Allocate raw memory
  void *allocatedMemory = core::memory::MemoryManager::Allocate(
      sizeof(vulkan::VulkanBackend), core::memory::MEMORY_TAG_RENDERER);

  if (!allocatedMemory) {
    throw std::runtime_error("Failed to allocate memory for VulkanBackend");
  }

  // Construct the VulkanBackend object in allocated memory
  IRendererBackend *instance = new (allocatedMemory) vulkan::VulkanBackend();

  // Wrap in unique_ptr with custom deleter
  auto backendPtr =
      core::memory::unique_stateful_renderer_ptr<IRendererBackend>(
          instance, core::memory::StatefulCustomDeleter<IRendererBackend>(
                        sizeof(vulkan::VulkanBackend),
                        core::memory::MEMORY_TAG_RENDERER));

  return backendPtr;
}

FrontendRenderer::FrontendRenderer(const string &applicationName,
                                   struct platform::PlatformState *platState) {

  CreateBackends(platState, _backends);
  // Set the active backend
  if (_backends[RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN]) {
    _activeBackend =
        _backends[RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN].get();
  } else {
    FFATAL("FrontendRenderer::FrontendRenderer(): No valid backend found.");
    throw std::runtime_error("No valid renderer backend could be initialized.");
  }

  // Initialize the backend
  if (!_activeBackend->Initialize(applicationName.c_str(), platState)) {
    FFATAL(
        "FrontendRenderer::FrontendRenderer(): Backend failed to initialize.");
    throw std::runtime_error("Failed to initialize backend.");
  }

  FINFO("FrontendRenderer::FrontendRenderer(): Successfully initialized.");

  /* OLD
  CreateBackend(RENDERER_BACKEND_TYPE_VULKAN, platState, _backend);
  _backend->frameNumber = 0;

  if (!_backend->Initialize(_backend, applicationName.c_str(), platState)) {
    FFATAL("FrontendRenderer::FrontendRenderer(): backend renderer failed to "
           "initialize.");
    throw std::runtime_error(
        "Failed to initialize backend renderer. Implementation missing?");
  }
  */
}

FrontendRenderer::~FrontendRenderer() {
  FINFO("FrontendRenderer::~FrontendRenderer(): shutting down frontend "
        "renderer...");
  _activeBackend = nullptr;
}

bool FrontendRenderer::BeginFrame(float32 deltaTime) {
  if (_activeBackend) {
    return _activeBackend->BeginFrame(deltaTime);
  }
  return FeFalse; // No active backend selected
}

bool FrontendRenderer::EndFrame(float32 deltaTime) {
  if (_activeBackend) {
    return _activeBackend->EndFrame(deltaTime);
  }
  return FeFalse; // No active backend selected
}

bool FrontendRenderer::DrawFrame(RenderPacket *packet) {
  if (!BeginFrame(packet->deltaTime)) {
    return FeFalse;
  }

  bool result = EndFrame(packet->deltaTime);
  if (!result) {
    FERROR(
        "FrontendRenderer::DrawFrame(): FrontendRenderer::EndFrame() failed");
    return FeFalse;
  }

  return FeTrue;
}

void FrontendRenderer::CreateBackends(
    struct platform::PlatformState *platState,
    core::memory::unique_stateful_renderer_ptr<IRendererBackend> (
        &backends)[MAX_BACKENDS]) {
  void *allocatedMemory = core::memory::MemoryManager::Allocate(
      sizeof(vulkan::VulkanBackend), core::memory::MEMORY_TAG_RENDERER);

  if (allocatedMemory) {
    backends[RendererBackendType::RENDERER_BACKEND_TYPE_VULKAN] =
        core::memory::unique_stateful_renderer_ptr<IRendererBackend>(
            new (allocatedMemory) vulkan::VulkanBackend(),
            core::memory::StatefulCustomDeleter<IRendererBackend>(
                sizeof(vulkan::VulkanBackend),
                core::memory::MEMORY_TAG_RENDERER));
  }
}

} // namespace renderer
} // namespace flatearth
