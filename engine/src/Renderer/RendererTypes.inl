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

constexpr int MAX_BACKENDS = 1;

typedef enum RendererBackendType {
  RENDERER_BACKEND_TYPE_VULKAN = 0,
  RENDERER_BACKEND_TYPE_OPENGL = 1,
  RENDERER_BACKEND_TYPE_DIRECTX = 2,
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

class IRendererBackend {
public:
  virtual ~IRendererBackend() = default;

  virtual bool Initialize(const char *applicationName,
                          struct platform::PlatformState *platState) = 0;

  virtual void Shutdown() = 0;
  virtual void OnResize(ushort width, ushort height) = 0;
  virtual bool BeginFrame(float32 deltaTime) = 0;
  virtual bool EndFrame(float32 deltaTime) = 0;
};

struct BackendArray {
  core::memory::unique_stateful_renderer_ptr<IRendererBackend>
      backends[MAX_BACKENDS];
};

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_RENDERER_TYPES_INL
