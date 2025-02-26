#include "VulkanDevice.hpp"
#include "Containers/DArray.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Vulkan/VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

#define MAX_QUEUE_TYPES 4

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

bool CreateDevice(Context *context) {
  if (!SelectPhysicalDevice(context)) {
    return FeFalse;
  }

  FINFO("CreateDevice(): Creating logical device...");

  // Get queue family to not mess up the indices
  uint32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(context->device.physicalDevice,
                                           &queueFamilyCount, nullptr);
  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(context->device.physicalDevice,
                                           &queueFamilyCount, queueFamilies);

  uint32 indices[MAX_QUEUE_TYPES];
  uint32 indexCount = 0;

#define ADD_UNIQUE_QUEUE_INDEX(queueIndex)                                     \
  do {                                                                         \
    bool exists = FeFalse;                                                     \
    for (uint32 j = 0; j < indexCount; j++) {                                  \
      if (indices[j] == queueIndex) {                                          \
        exists = FeTrue;                                                       \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!exists && queueIndex < queueFamilyCount) {                            \
      indices[indexCount++] = queueIndex;                                      \
    }                                                                          \
  } while (0)

  ADD_UNIQUE_QUEUE_INDEX(context->device.graphicsQueueIndex);
  ADD_UNIQUE_QUEUE_INDEX(context->device.presentQueueIndex);
  ADD_UNIQUE_QUEUE_INDEX(context->device.transferQueueIndex);

  VkDeviceQueueCreateInfo queueCreateInfo[indexCount];
  for (uint32 i = 0; i < indexCount; i++) {
    queueCreateInfo[i].sType = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo[i].queueFamilyIndex = indices[i];

    uint32 maxQueues = queueFamilies[indices[i]].queueCount;

    queueCreateInfo[i].queueCount =
        (indices[i] == context->device.graphicsQueueIndex && maxQueues >= 2)
            ? 2
            : 1;

    queueCreateInfo[i].queueCount = queueCreateInfo[i].queueCount > maxQueues
                                        ? maxQueues
                                        : queueCreateInfo[i].queueCount;

    queueCreateInfo[i].flags = 0;
    queueCreateInfo[i].pNext = nullptr;
    float32 queuePriority = 1.0f;
    queueCreateInfo[i].pQueuePriorities = &queuePriority;
  }

  // TODO: make this not hardcoded
  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  deviceCreateInfo.queueCreateInfoCount = indexCount;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount = 1;
  const char *extNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  deviceCreateInfo.ppEnabledExtensionNames = &extNames;
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;

  VK_CHECK(vkCreateDevice(context->device.physicalDevice, &deviceCreateInfo,
                          context->allocator, &context->device.logicalDevice));

  FINFO("CreateDevice(): Logical device created");

  // Queue selection
  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.graphicsQueueIndex, 0,
                   &context->device.graphicsQueue);
  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.presentQueueIndex, 0,
                   &context->device.presentQueue);
  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.transferQueueIndex, 0,
                   &context->device.transferQueue);

  FINFO("CreateDevice(): Queues obtained");

  return FeTrue;
}

void DestroyDevice(Context *context) {
  context->device.graphicsQueue = nullptr;
  context->device.presentQueue = nullptr;
  context->device.transferQueue = nullptr;

  if (context->device.logicalDevice) {
    FINFO("DestroyDevice(): Destroying logical device...");
    vkDestroyDevice(context->device.logicalDevice, context->allocator);
    context->device.logicalDevice = nullptr;
    FINFO("DestroyDevice(): Logical device destroyed");
  }

  // Releasing resources on physical device
  FINFO("DestroyDevice(): Releasing physical device resources...");
  context->device.physicalDevice = nullptr;
  // No need to free resources because they are smart pointers
  context->device.swapchainSupport.formatCount = 0;
  context->device.swapchainSupport.presentModeCount = 0;

  core::memory::MemoryManager::ZeroMemory(
      &context->device.swapchainSupport.capabilities,
      sizeof(context->device.swapchainSupport.capabilities));

  context->device.graphicsQueueIndex = -1;
  context->device.presentQueueIndex = -1;
  context->device.transferQueueIndex = -1;
}

void QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface,
                           SwapchainSupportInfo *outSwapchainInfo) {
  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &outSwapchainInfo->capabilities));

  // Surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &outSwapchainInfo->formatCount, nullptr));

  if (outSwapchainInfo->formatCount != 0) {
    if (!outSwapchainInfo->formats) {
      outSwapchainInfo->formats = unique_renderer_ptr<VkSurfaceFormatKHR>(
          static_cast<VkSurfaceFormatKHR *>(
              core::memory::MemoryManager::Allocate(
                  sizeof(VkSurfaceFormatKHR) * outSwapchainInfo->formatCount,
                  core::memory::MEMORY_TAG_RENDERER)));
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &outSwapchainInfo->formatCount,
        outSwapchainInfo->formats.get()));
  }

  // Present mode
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &outSwapchainInfo->presentModeCount, nullptr));
  if (outSwapchainInfo->presentModeCount != 0) {
    if (!outSwapchainInfo->presentMode) {
      outSwapchainInfo->presentMode = unique_renderer_ptr<VkPresentModeKHR>(
          static_cast<VkPresentModeKHR *>(core::memory::MemoryManager::Allocate(
              sizeof(VkPresentModeKHR) * outSwapchainInfo->presentModeCount,
              core::memory::MEMORY_TAG_RENDERER)));
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &outSwapchainInfo->presentModeCount,
        outSwapchainInfo->presentMode.get()));
  }
}

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

    if (!result) {
      FERROR("SelectPhysicalDevice(): No physical devices were found which "
             "meet the requirements");
      return FeFalse;
    }

    FINFO("Selected device: '%s'", properties.deviceName);
    switch (properties.deviceType) {
    default:
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      FINFO("GPU type is unknown");
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      FINFO("GPU type is integrated");
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      FINFO("GPU type is discrete");
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      FINFO("GPU type is virtual");
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      FINFO("GPU type is CPU");
      break;
    }

    FINFO("GPU Driver version: %d.%d.%d",
          VK_VERSION_MAJOR(properties.driverVersion),
          VK_VERSION_MINOR(properties.driverVersion),
          VK_VERSION_PATCH(properties.driverVersion));

    // Vulkan API version
    FINFO("Vulkan API version: %d.%d.%d",
          VK_VERSION_MAJOR(properties.apiVersion),
          VK_VERSION_MINOR(properties.apiVersion),
          VK_VERSION_PATCH(properties.apiVersion));

    // Memory informations
    for (uint32 j = 0; j < memory.memoryHeapCount; j++) {
      float32 memorySizeGib =
          (((float32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);

      if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        FINFO("Local GPU memory: %.2f GiB", memorySizeGib);
      } else {
        FINFO("Shared System memory: %.2f GiB", memorySizeGib);
      }
    }

    context->device.physicalDevice = physicalDevices[i];
    context->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
    context->device.presentQueueIndex = queueInfo.presentFamilyIndex;
    context->device.transferQueueIndex = queueInfo.transferFamilyIndex;

    // Keep a copy of properties, features and memory for later use
    context->device.properties = properties;
    context->device.features = features;
    context->device.memory = memory;
    break;
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
        outQueueInfo->graphicsFamilyIndex, outQueueInfo->presentFamilyIndex,
        outQueueInfo->computeFamilyIndex, outQueueInfo->transferFamilyIndex,
        properties->deviceName);

  if ((!requirements->graphics ||
       (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
      (!requirements->present ||
       (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
      (!requirements->compute ||
       (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
      (!requirements->transfer ||
       (requirements->transfer && outQueueInfo->transferFamilyIndex != -1))) {
    FINFO("Device meets all queue requirements");
    FTRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex);
    FTRACE("Presentation Family Index: %i", outQueueInfo->presentFamilyIndex);
    FTRACE("Compute Family Index: %i", outQueueInfo->computeFamilyIndex);
    FTRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex);

    QuerySwapchainSupport(device, surface, outSwapchainInfo);

    if (outSwapchainInfo->formatCount < 1 ||
        outSwapchainInfo->presentModeCount < 1) {
      FINFO("Required swapchain support not present, skipping device.");
      return FeFalse;
    }

    // Device extensions
    if (!requirements->deviceExtNames.IsEmpty()) {
      uint32 availableExtCount = 0;
      unique_renderer_ptr<VkExtensionProperties> availableExts;
      VK_CHECK(vkEnumerateDeviceExtensionProperties(
          device, nullptr, &availableExtCount, nullptr));
      if (availableExtCount != 0) {
        availableExts = unique_renderer_ptr<VkExtensionProperties>(
            static_cast<VkExtensionProperties *>(
                core::memory::MemoryManager::Allocate(
                    sizeof(VkExtensionProperties) * availableExtCount,
                    core::memory::MEMORY_TAG_RENDERER)));
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            device, nullptr, &availableExtCount, availableExts.get()));

        uint32 requiredExtCount = requirements->deviceExtNames.GetLength();
        for (uint32 i = 0; i < requiredExtCount; i++) {
          bool found = FeFalse;
          for (uint32 j = 0; j < availableExtCount; j++) {
            if (std::strcmp(requirements->deviceExtNames[i],
                            availableExts.get()[j].extensionName)) {
              found = FeTrue;
              break;
            }
          }

          if (!found) {
            FINFO("Required extension not found: '%s', skipping device.",
                  requirements->deviceExtNames[i]);
            return FeFalse;
          }
        }
      }
    }

    // Sampler anisotropy
    if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
      FINFO("Device does not support sampler anisotropy, skipping.");
      return FeFalse;
    }

    return FeTrue;
  }

  // Reaching here means we did not find any device at all
  return FeFalse;
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
