#ifndef _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP
#define _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP

#include "Core/FeMemory.hpp"
#include "RendererTypes.inl"

namespace flatearth {
namespace renderer {

bool CreateBackend(
    RendererBackendType type, struct platform::PlatformState *platState,
    core::memory::unique_renderer_ptr<RendererBackend> &outRenderer);

void DestroyBackend(
    core::memory::unique_renderer_ptr<RendererBackend> &renderer);

} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_RENDERER_BACKEND_HPP
