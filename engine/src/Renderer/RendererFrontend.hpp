#ifndef _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
#define _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP

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
  unique_backend_renderer_ptr _backend;
};

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARHT_ENGINE_RENDERER_FRONTEND_HPP
