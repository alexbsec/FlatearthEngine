#include "RendererFrontend.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Renderer/RendererTypes.inl"
#include "RendererBackend.hpp"
#include <stdexcept>

namespace flatearth {
namespace renderer {

FrontendRenderer::FrontendRenderer(const string &applicationName,
                                   struct platform::PlatformState *platState) {
  RendererBackend *allocatedMemory =
      reinterpret_cast<RendererBackend *>(core::memory::MemoryManager::Allocate(
          sizeof(RendererBackend), core::memory::MEMORY_TAG_RENDERER));

  _backend = unique_backend_renderer_ptr(
      allocatedMemory,
      core::memory::StatelessCustomDeleter<
          RendererBackend, 1, core::memory::MEMORY_TAG_RENDERER>());

  CreateBackend(RENDERER_BACKEND_TYPE_VULKAN, platState, _backend);
  _backend->frameNumber = 0;

  if (!_backend->Initialize(_backend, applicationName.c_str(), platState)) {
    FFATAL("FrontendRenderer::FrontendRenderer(): backend renderer failed to "
           "initialize.");
    throw std::runtime_error(
        "Failed to initialize backend renderer. Implementation missing?");
  }

  FINFO("FrontendRenderer::FrontendRenderer(): front end renderer was "
        "correctly initialized");
}

FrontendRenderer::~FrontendRenderer() {
  FINFO("FrontendRenderer::~FrontendRenderer(): shutting down frontend "
        "renderer...");
  _backend->Shutdown(_backend);
}

bool FrontendRenderer::BeginFrame(float32 deltaTime) {
  return _backend->BeginFrame(_backend, deltaTime);
}

bool FrontendRenderer::EndFrame(float32 deltaTime) {
  return _backend->EndFrame(_backend, deltaTime);
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

} // namespace renderer
} // namespace flatearth
