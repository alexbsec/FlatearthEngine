#ifndef _FLATEARTH_ENGINE_VULKAN_TYPES_INL
#define _FLATEARTH_ENGINE_VULKAN_TYPES_INL

#include "Core/Asserts.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Definitions.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

#define VK_CHECK(expr) {FASSERT(expr == VK_SUCCESS)}

struct SwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  uint32 formatCount;
  VkSurfaceFormatKHR *formats;
  uint32 presentModeCount;
  VkPresentModeKHR *presentMode;
};

struct Device {
  VkPhysicalDevice physicalDevice;
  VkDevice logicalDevice;
  SwapchainSupportInfo swapchainSupport;
  sint32 graphicsQueueIndex;
  sint32 presentQueueIndex;
  sint32 transferQueueIndex;

  VkFormat depthFormat;

  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;

  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
};

struct Image {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  uint32 width;
  uint32 height;
};

struct Swapchain {
  VkSurfaceFormatKHR imageFormat;
  uchar maxFrames;
  VkSwapchainKHR handle;
  uint32 imageCount;
  VkImage *images;
  VkImageView *views;
  Image depthAttachment;
};

struct Context {
  uint32 framebufferWidth;
  uint32 framebufferHeight;
  uint32 currentFrame;
  uint32 imageIndex;

  VkInstance instance;
  VkAllocationCallbacks *allocator;
  VkSurfaceKHR surface;
  Device device;
  Swapchain swapchain;

  sint32 FindMemoryIndex(uint32 typeFilter, uint32 propertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memoryProps);

    for (uint32 i = 0; i < memoryProps.memoryTypeCount; i++) {
      if (typeFilter & (1 << i) &&
          (memoryProps.memoryTypes[i].propertyFlags & propertyFlags)) {
        return i;
      }
    }

    FWARN("Context::FindMemoryIndex(): Unable to find memory type");
    return -1;
  }

#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debugMessenger;

#endif
};

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_TYPES_INL
