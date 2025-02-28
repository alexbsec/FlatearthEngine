#ifndef _FLATEARTH_ENGINE_VULKAN_RENDER_PASS
#define _FLATEARTH_ENGINE_VULKAN_RENDER_PASS

#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

void CreateRenderPass(Context *context, RenderPass *outRenderPass, float32 x,
                      float32 y, float32 width, float32 height, float32 r,
                      float32 g, float32 b, float32 a, float32 depth,
                      uint32 stencil);

void DestroyRenderPass(Context *context, RenderPass *renderPass);

void BeginRenderPass(CommandBuffer *cmdBuffer, RenderPass *renderPass,
                     VkFramebuffer frameBuffer);

void EndRenderPass(CommandBuffer *cmdBuffer, RenderPass *renderPass);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_RENDER_PASS
