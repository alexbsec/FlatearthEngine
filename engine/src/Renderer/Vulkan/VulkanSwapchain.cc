#include "VulkanSwapchain.hpp"
#include "Core/Logger.hpp"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

// Private functions to wrap up logic
void Create(Context *context, uint32 width, uint32 height,
            Swapchain *outSwapchain);
void Destroy(Context *context, Swapchain *swapchain);

// Actual implementation of the member functions

void CreateSwapchain(Context *context, uint32 width, uint32 height,
                     Swapchain *outSwapchain) {
  Create(context, width, height, outSwapchain);
}

void RecreateSwapchain(Context *context, uint32 width, uint32 height,
                       Swapchain *outSwapchain) {
  // Destroys the old one and creates a new one
  Destroy(context, outSwapchain);
  Create(context, width, height, outSwapchain);
}

void DestroySwapchain(Context *context, Swapchain *swapchain) {
  Destroy(context, swapchain);
}

bool AcquireSwapchainNextImageIndex(Context *context, Swapchain *swapchain,
                                    uint64 timeoutns,
                                    VkSemaphore imageAvailableSemaphore,
                                    VkFence fence, uint32 *outImageIndex) {
  VkResult result = vkAcquireNextImageKHR(
      context->device.logicalDevice, swapchain->handle, timeoutns,
      imageAvailableSemaphore, fence, outImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // This errors happens when resizing goes wrong for example
    // We simply recreate the swapchain in this case
    RecreateSwapchain(context, context->framebufferWidth,
                      context->framebufferHeight, swapchain);
    return FeFalse;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    // These are only Vulkan errors and we simply log a fatal error
    // for the engine
    FFATAL(
        "AcquireSwapchainNextImageIndex(): Failed to acquire swapchain image");
    return FeFalse;
  }

  // If hits here, it means everything is ok
  return FeTrue;
}

void PresentSwapchain(Context *context, Swapchain *swapchain,
                      VkQueue graphicsQueue, VkQueue presentQueue,
                      VkSemaphore renderCompleteSemaphore,
                      uint32 presentImageIndex) {
  // Return the image to the swapchain for presentation
  VkPresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain->handle;
  presentInfo.pImageIndices = &presentImageIndex;
  presentInfo.pResults = nullptr;

  VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    // Must recreate
    RecreateSwapchain(context, context->framebufferWidth,
                      context->framebufferHeight, swapchain);
  } else if (result != VK_SUCCESS) {
    FFATAL("PresentSwapchain(): Failed to present swapchain image");
  }
}

void Create(Context *context, uint32 width, uint32 height, Swapchain *swapchain) {}

void Destroy(Context *context, Swapchain *swapchain) {}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
