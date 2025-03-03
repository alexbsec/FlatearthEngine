#ifndef _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
#define _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP

#include "Core/FeMemory.hpp"
#include "RendererTypes.inl"

namespace flatearth {
namespace renderer {

struct StaticMeshData;

class FrontendRenderer {
public:
  FrontendRenderer(const string &applicationName,
                   struct platform::PlatformState *platState);
  ~FrontendRenderer();

  bool BeginFrame(float32 deltaTime);
  bool EndFrame(float32 deltaTime);
  bool DrawFrame(RenderPacket *packet);
  void OnResize(ushort width, ushort height);

private:
  static void
  CreateBackends(struct platform::PlatformState *platState,
                 core::memory::unique_stateful_renderer_ptr<IRendererBackend> (
                     &backends)[MAX_BACKENDS]);

  core::memory::unique_stateful_renderer_ptr<IRendererBackend>
      _backends[MAX_BACKENDS];
  IRendererBackend *_activeBackend;
};

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
