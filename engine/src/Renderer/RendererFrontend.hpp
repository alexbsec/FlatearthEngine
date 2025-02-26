#ifndef _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
#define _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP

#include "RendererTypes.inl"
#include "Core/FeMemory.hpp"

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
  core::memory::unique_renderer_ptr<RendererBackend> _backend;
};

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
