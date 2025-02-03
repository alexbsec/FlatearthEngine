#ifndef _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP
#define _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP

#include "Core/FeMemory.hpp"
#include "RendererTypes.inl"

namespace flatearth {
namespace renderer {

bool CreateBackend(RendererBackendType type,
                   struct platform::PlatformState *platState,
                   unique_backend_renderer_ptr &outRenderer);

void DestroyBackend(unique_backend_renderer_ptr &renderer);

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP
