#include "VulkanSwapchain.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Vulkan/VulkanDevice.hpp"
#include <utility>
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

void Create(Context *context, uint32 width, uint32 height,
            Swapchain *outSwapchain) {
  VkExtent2D swapchainExtent = {width, height};
  outSwapchain->maxFrames = 2;

  bool found = FeFalse;
  for (uint32 i = 0; i < context->device.swapchainSupport.formatCount; i++) {
    VkSurfaceFormatKHR format = context->device.swapchainSupport.formats[i];
    // Preferred formats
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      outSwapchain->imageFormat = format;
      found = FeTrue;
      break;
    }
  }

  if (!found) {
    // Take first if preferred is not found
    outSwapchain->imageFormat = context->device.swapchainSupport.formats[0];
  }

  // Set default as FIFO, but preferred is mailbox
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (uint32 i = 0; i < context->device.swapchainSupport.presentModeCount;
       i++) {
    VkPresentModeKHR mode = context->device.swapchainSupport.presentMode[i];
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = mode;
      break;
    }
  }

  // Requery swapchain support
  QuerySwapchainSupport(context->device.physicalDevice, context->surface,
                        &context->device.swapchainSupport);

  // Swapchain support capabilities
  if (context->device.swapchainSupport.capabilities.currentExtent.width !=
      UINT32_MAX) {
    swapchainExtent =
        context->device.swapchainSupport.capabilities.currentExtent;
  }

  // Clamp to an allowed value by the GPU
  VkExtent2D min = context->device.swapchainSupport.capabilities.minImageExtent;
  VkExtent2D max = context->device.swapchainSupport.capabilities.maxImageExtent;
  swapchainExtent.width = FCLAMP(swapchainExtent.width, min.width, max.width);
  swapchainExtent.height =
      FCLAMP(swapchainExtent.height, min.height, max.height);

  uint32 imageCount =
      context->device.swapchainSupport.capabilities.minImageCount + 1;
  if (context->device.swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount >
          context->device.swapchainSupport.capabilities.maxImageCount) {
    // Safegard clamp
    imageCount = context->device.swapchainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchainCreateInfo.surface = context->surface;
  swapchainCreateInfo.minImageCount = imageCount;
  swapchainCreateInfo.imageFormat = outSwapchain->imageFormat.format;
  swapchainCreateInfo.imageColorSpace = outSwapchain->imageFormat.colorSpace;
  swapchainCreateInfo.imageExtent = swapchainExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Setup the queue family indices
  if (context->device.graphicsQueueIndex != context->device.presentQueueIndex) {
    uint32 queueFamilyIndices[] = {(uint32)context->device.graphicsQueueIndex,
                                   (uint32)context->device.presentQueueIndex};
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  swapchainCreateInfo.preTransform =
      context->device.swapchainSupport.capabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = presentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = nullptr;

  // Create the swapchain and store it
  VK_CHECK(vkCreateSwapchainKHR(context->device.logicalDevice,
                                &swapchainCreateInfo, context->allocator,
                                &outSwapchain->handle));

  // Starts with zero frames
  context->currentFrame = 0;

  // Setting images
  outSwapchain->imageCount = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice,
                                   outSwapchain->handle,
                                   &outSwapchain->imageCount, nullptr));
  // Allocate memory if not already
  if (!outSwapchain->images) {
    outSwapchain->images = (VkImage *)core::memory::MemoryManager::Allocate(
        sizeof(VkImage) * outSwapchain->imageCount,
        core::memory::MEMORY_TAG_RENDERER);
  }

  if (!outSwapchain->views) {
    outSwapchain->views = (VkImageView *)core::memory::MemoryManager::Allocate(
        sizeof(VkImageView) * outSwapchain->imageCount,
        core::memory::MEMORY_TAG_RENDERER);
  }

  // Assign images
  VK_CHECK(vkGetSwapchainImagesKHR(
      context->device.logicalDevice, outSwapchain->handle,
      &outSwapchain->imageCount, outSwapchain->images));

  // Setting views
  for (uint32 i = 0; i < outSwapchain->imageCount; i++) {
    VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = outSwapchain->images[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = outSwapchain->imageFormat.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    // Assign view
    VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewInfo,
                               context->allocator, &outSwapchain->views[i]));
  }

  // TODO: depth features and create image 
}

void Destroy(Context *context, Swapchain *swapchain) {}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
