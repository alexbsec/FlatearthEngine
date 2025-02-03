#ifndef _FLATEARTH_ENGINE_RENDERER_TYPES_INL
#define _FLATEARTH_ENGINE_RENDERER_TYPES_INL

#include "Core/FeMemory.hpp"
#include "Definitions.hpp"
#include "Platform/Platform.hpp"
#include <functional>

namespace flatearth {
namespace renderer {

struct RendererBackend;

using unique_backend_renderer_ptr =
    std::unique_ptr<RendererBackend,
                    core::memory::StatelessCustomDeleter<
                        RendererBackend, 1, core::memory::MEMORY_TAG_RENDERER>>;

typedef enum RendererBackendType {
  RENDERER_BACKEND_TYPE_VULKAN,
  RENDERER_BACKEND_TYPE_OPENGL,
  RENDERER_BACKEND_TYPE_DIRECTX,
} RendererBackendType;

struct RendererBackend {
  struct platform::PlatformState *platState;
  uint64 frameNumber;

  std::function<bool(unique_backend_renderer_ptr &backend,
                     const char *applicationName,
                     struct platform::PlatformState *platState)>
      Initialize;

  std::function<void(unique_backend_renderer_ptr &backend)> Shutdown;

  std::function<void(unique_backend_renderer_ptr &backend, ushort width,
                     ushort height)>
      OnResize;

  std::function<bool(unique_backend_renderer_ptr &backend, float32 deltaTime)>
      BeginFrame;

  std::function<bool(unique_backend_renderer_ptr &backend, float32 deltaTime)>
      EndFrame;
};

struct RenderPacket {
  float32 deltaTime;
};

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_RENDERER_TYPES_INL
