#ifndef _FLATEARTH_ENGINE_VULKAN_SWAPCHAIN_HPP
#define _FLATEARTH_ENGINE_VULKAN_SWAPCHAIN_HPP

#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

void CreateSwapchain(Context *context, uint32 width, uint32 height,
                     Swapchain *outSwapchain);

void RecreateSwapchain(Context *context, uint32 width, uint32 height,
                       Swapchain *outSwapchain);

void DestroySwapchain(Context *context, Swapchain *swapchain);

bool AcquireSwapchainNextImageIndex(Context *context, Swapchain *swapchain,
                                    uint64 timeoutns,
                                    VkSemaphore imageAvailableSemaphore,
                                    VkFence fence, uint32 *outImageIndex);

void PresentSwapchain(Context *context, Swapchain *swapchain,
                      VkQueue graphicsQueue, VkQueue presentQueue,
                      VkSemaphore renderCompleteSemaphore,
                      uint32 presentImageIndex);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_SWAPCHAIN_HPP
