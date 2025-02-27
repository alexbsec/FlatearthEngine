#ifndef _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP
#define _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP

#include "VulkanTypes.inl"

namespace flatearth {
namespace renderer {
namespace vulkan {

bool CreateDevice(Context *context);

void DestroyDevice(Context *context);

void QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface,
                           SwapchainSupportInfo *outSwapchainInfo);

bool DetectDeviceDepthFormat(Device *device);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP
