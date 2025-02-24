#include "VulkanDevice.hpp"
#include "Containers/DArray.hpp"
#include "Core/Logger.hpp"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

// Private structures
struct PhysicalDeviceRequirements {
  bool graphics;
  bool present;
  bool compute;
  bool transfer;
  containers::DArray<const char *> deviceExtNames;
  bool samplerAnisotropy;
  bool discreteGPU;
};

struct PhysicalDeviceQueueFamilyInfo {
  uint32 graphicsFamilyIndex;
  uint32 presentFamilyIndex;
  uint32 computeFamilyIndex;
  uint32 transferFamilyIndex;
};

// Private methods

bool SelectPhysicalDevice(Context *context);

bool PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties *properties,
    const VkPhysicalDeviceFeatures *features,
    const PhysicalDeviceRequirements *requirements,
    PhysicalDeviceQueueFamilyInfo *outQueueInfo,
    SwapchainSupportInfo *outSwapchainInfo);

bool CreateDevice(Context *context) { return SelectPhysicalDevice(context); }

void DestroyDevice(Context *context) {}

bool SelectPhysicalDevice(Context *context) {
  uint32 physicalDeviceCount = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount,
                                      nullptr));
  if (physicalDeviceCount == 0) {
    FFATAL(
        "SelectPhysicalDevice(): No device which support Vulkan were found!");
    return FeFalse;
  }

  VkPhysicalDevice physicalDevices[physicalDeviceCount];
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount,
                                      physicalDevices));
  for (uint32 i = 0; i < physicalDeviceCount; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

    // TODO: smart select based on the engine
    PhysicalDeviceRequirements requirements = {};
    requirements.graphics = FeTrue;
    requirements.present = FeTrue;
    requirements.transfer = FeTrue;
    requirements.compute = FeFalse;
    requirements.samplerAnisotropy = FeTrue;
    requirements.discreteGPU = FeFalse;
    requirements.deviceExtNames.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    PhysicalDeviceQueueFamilyInfo queueInfo = {};
    bool result = PhysicalDeviceMeetsRequirements(
        physicalDevices[i], context->surface, &properties, &features,
        &requirements, &queueInfo, &context->device.swapchainSupport);
  }

  return FeTrue;
}

bool PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties *properties,
    const VkPhysicalDeviceFeatures *features,
    const PhysicalDeviceRequirements *requirements,
    PhysicalDeviceQueueFamilyInfo *outQueueInfo,
    SwapchainSupportInfo *outSwapchainInfo) {
  // Initialize them at -1 saying it does not meet requirements
  outQueueInfo->graphicsFamilyIndex = -1;
  outQueueInfo->computeFamilyIndex = -1;
  outQueueInfo->presentFamilyIndex = -1;
  outQueueInfo->transferFamilyIndex = -1;

  // Check whether working on a discrete GPU
  if (requirements->discreteGPU) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      FINFO("PhysicalDeviceMeetsRequirements(): Device is not a discrete GPU, "
            "and one is required. Skipping.");
      return FeFalse;
    }
  }

  uint32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies);

  // Look at each queue and see what queues it supports
  FINFO("Graphics | Present | Compute | Transfer | Name");
  uchar minTransferScore = 255;
  for (uint32 i = 0; i < queueFamilyCount; i++) {
    uchar currentTransferScore = 0;

    // Check if graphics queue is supported
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      outQueueInfo->graphicsFamilyIndex = i;
      currentTransferScore++;
    }

    // Check if compute queue is supported
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      outQueueInfo->computeFamilyIndex = i;
      currentTransferScore++;
    }

    // Check whether transfer queue is supported
    if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      if (currentTransferScore <= minTransferScore) {
        minTransferScore = currentTransferScore;
        outQueueInfo->transferFamilyIndex = i;
      }
    }

    // Finally, check whether present queue is supported
    VkBool32 supportsPresent = VK_FALSE;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                                  &supportsPresent));
    if (supportsPresent) {
      outQueueInfo->presentFamilyIndex = i;
    }
  }

  FINFO("       %d |       %d |       %d |        %d | %s",
        outQueueInfo->graphicsFamilyIndex = -1,
        outQueueInfo->presentFamilyIndex = -1,
        outQueueInfo->computeFamilyIndex = -1,
        outQueueInfo->transferFamilyIndex = -1, properties->deviceName);

  // TODO: compare against what is required
  
  return FeTrue;
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
