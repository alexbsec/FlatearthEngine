#include "VulkanBackend.hpp"

#include "Containers/DArray.hpp"
#include "Core/Application.hpp"
#include "Core/FeMemory.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanTypes.inl"
#include "VulkanUtils.hpp"
#include <alloca.h>
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

static uint32 cachedFrameBufferWidth = 0;
static uint32 cachedFrameBufferHeight = 0;

#define MAX_QUEUE_TYPES 4

// Forward declaration

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, uint32 messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData);

/******************* VULKAN BACKEND CLASS IMPLEMENTATION **********************/

VulkanBackend::~VulkanBackend() {
  FINFO("VulkanBackend::~VulkanBackend(): shutting down Vulkan backend...");
  Shutdown();
}

bool VulkanBackend::Initialize(const char *applicationName,
                               struct platform::PlatformState *platState) {
  // TODO: custom allocator
  _context.allocator = nullptr;

  core::application::App::GetFrameBufferSize(&cachedFrameBufferWidth,
                                             &cachedFrameBufferHeight);
  _context.framebufferWidth =
      (cachedFrameBufferWidth != 0) ? cachedFrameBufferWidth : 800;
  _context.framebufferHeight =
      (cachedFrameBufferHeight != 0) ? cachedFrameBufferHeight : 600;

  // Reset framebuffer cache
  cachedFrameBufferWidth = 0;
  cachedFrameBufferHeight = 0;

  VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion = VK_API_VERSION_1_2;
  appInfo.pApplicationName = applicationName;
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Flatearth Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo = &appInfo;

  // Obtain list of required extensions
  containers::DArray<const char *> requiredExtensions;
  requiredExtensions.Push(VK_KHR_SURFACE_EXTENSION_NAME);

  // Platform specific extensions
  platform::GetRequiredExtNames(&requiredExtensions);

#if defined(_DEBUG)
  requiredExtensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  FDEBUG("Required extensions:");
  uint32 len = requiredExtensions.GetLength();
  for (uint32 i = 0; i < len; i++) {
    FDEBUG(requiredExtensions[i]);
  }
#endif

  createInfo.enabledExtensionCount = requiredExtensions.GetLength();
  createInfo.ppEnabledExtensionNames = requiredExtensions.Data();

  // Validation layers
  containers::DArray<const char *> requiredValidationLayerNames;
  uint32 requiredValidationLayerCount = 0;

#if defined(_DEBUG)
  FINFO("Validation layers enabled. Enumerating...");

  requiredValidationLayerNames.Push("VK_LAYER_KHRONOS_validation");
  requiredValidationLayerCount = requiredValidationLayerNames.GetLength();

  // Obtain list of available layers
  uint32 availableLayerCount = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
  containers::DArray<VkLayerProperties> availableLayers;
  availableLayers.Reserve(availableLayerCount);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount,
                                              availableLayers.Data()));

  // Verify if all layers are available
  for (uint32 i = 0; i < requiredValidationLayerCount; i++) {
    FINFO("Searching for layer: %s...", requiredValidationLayerNames[i]);
    bool found = FeFalse;
    for (uint32 j = 0; j < availableLayerCount; j++) {
      string reqValLayerStr(requiredValidationLayerNames[i]);
      string availableLayerStr(availableLayers[j].layerName);
      if (reqValLayerStr == availableLayerStr) {
        found = FeTrue;
        FINFO("Found!");
        break;
      }
    }

    if (!found) {
      FFATAL("Required validation layer is missing: %s",
             requiredValidationLayerNames[i]);
      return FeFalse;
    }
  }

  FINFO("All required validation layers were found");
#endif

  createInfo.enabledLayerCount = requiredValidationLayerCount;
  createInfo.ppEnabledLayerNames = requiredValidationLayerNames.Data();

  VK_CHECK(
      vkCreateInstance(&createInfo, _context.allocator, &_context.instance));
  FINFO("VulkanBackend::Initialize(): Vulkan instance created");

  // Vulkan debugger
#if defined(_DEBUG)
  FDEBUG("Creating Vulkan debugger...");
  uint32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

  debugCreateInfo.messageSeverity = logSeverity;
  debugCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugCreateInfo.pfnUserCallback = DebugCallback;

  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _context.instance, "vkCreateDebugUtilsMessengerEXT");
  FASSERT_MSG(func, "Failed to create debug messenger");
  VK_CHECK(func(_context.instance, &debugCreateInfo, _context.allocator,
                &_context.debugMessenger));
  FDEBUG("Vulkan debugger created");
#endif

  // Surface creation
  FDEBUG("Creating Vulkan surface...");
  if (!platform::CreateVulkanSurface(platState, &_context)) {
    FERROR("VulkanBackend::Initialize(): Failed to create Vulkan surface");
    return FeFalse;
  }
  FDEBUG("Vulkan surface created");

  if (!DeviceCreate()) {
    FERROR("VulkanBackend::Initialize(): Failed to create device");
    return FeFalse;
  }

  // Swapchain creation
  FDEBUG("VulkanBackend::Initialize(): Creating swapchain...");
  SwapchainCreate(_context.framebufferWidth, _context.framebufferHeight,
                  &_context.swapchain);
  FINFO("IntializeVulkan(): Swapchain created successfully");

  // Render pass creation
  FDEBUG("VulkanBackend::Initialize(): Creating render pass...");
  RenderPassCreate(&_context.mainRenderPass, 0, 0, _context.framebufferWidth,
                   _context.framebufferHeight, 0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 0);
  FINFO("VulkanBackend::Initialize(): Render pass created successfully");

  // Framebuffer creation
  FDEBUG("VulkanBackend::Initialize(): Regenerating framebuffers...");
  _context.swapchain.framebuffers.Reserve(_context.swapchain.imageCount);
  FrameBufferRegenerate(&_context.swapchain, &_context.mainRenderPass);
  FINFO("VulkanBackend::Initialize(): Framebuffers regenerated successfully");

  FDEBUG("VulkanBackend::Initialize(): Creating command buffers...");
  CommandBuffersCreate();
  FINFO("VulkanBackend::Initialize(): Command buffers created successfully");

  // Sync object creation
  FDEBUG("VulkanBackend::Initialize(): Creating sync objects & fences...");
  _context.imageAvailableSemaphores.Reserve(_context.swapchain.maxFrames);
  _context.queueCompleteSemaphores.Reserve(_context.swapchain.maxFrames);
  _context.inFlightFences.Reserve(_context.swapchain.maxFrames);
  for (uint32 i = 0; i < _context.swapchain.maxFrames; i++) {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;
    VK_CHECK(vkCreateSemaphore(_context.device.logicalDevice,
                               &semaphoreCreateInfo, _context.allocator,
                               &_context.imageAvailableSemaphores[i]));
    VK_CHECK(vkCreateSemaphore(_context.device.logicalDevice,
                               &semaphoreCreateInfo, _context.allocator,
                               &_context.queueCompleteSemaphores[i]));

    FenceCreate(FeTrue, &_context.inFlightFences[i]);
  }

  // At this point no in flight fences exist, so clear the array. These
  // are stored in pointers because initial state must be nullptr and will
  // be nullptr when not in use. Actual fences are not owned by this array
  _context.imagesInFlight.Reserve(_context.swapchain.imageCount);
  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    _context.imagesInFlight[i] = nullptr;
  }
  FINFO("VulkanBackend::Initialize(): Sync objects & fences created "
        "successfully");

  FINFO(
      "VulkanBackend::Initialize(): Vulkan renderer successfully initialized");

  return FeTrue;
}

void VulkanBackend::OnResize(ushort width, ushort height) {
  cachedFrameBufferWidth = width;
  cachedFrameBufferHeight = height;
  _context.framebufferSizeGeneration++;
  FINFO("VulkanBackend::OnResize(): w/h/gen: %i/%i/%llu", width, height,
        _context.framebufferSizeGeneration);
}

bool VulkanBackend::BeginFrame(float32 deltaTime) {
  // Handle to make it easier to call device
  Device *device = &_context.device;

  // Check if recreating swapchain is happening
  if (_context.recreatingSwapchain) {
    VkResult result = vkDeviceWaitIdle(device->logicalDevice);
    if (!utils::VkResultIsSuccess(result)) {
      FERROR("VulkanBackend::BeginFrame(): vkDeviceWaitIdle (1) failed: '%s'",
             utils::VkResultToString(result, FeTrue));
      return FeFalse;
    }

    FINFO("VulkanBackend::BeginFrame(): Recreating swapchain, skipping.");
    return FeFalse;
  }

  // Check if framebuffer has been resized. If so, a new swapchain must be
  // created
  if (_context.framebufferSizeGeneration !=
      _context.framebufferSizeLastGeneration) {
    VkResult result = vkDeviceWaitIdle(device->logicalDevice);
    if (!utils::VkResultIsSuccess(result)) {
      FERROR("VulkanBackend::BeginFrame(): vkDeviceWaitIdle (2) failed: '%s'",
             utils::VkResultToString(result, FeTrue));
      return FeFalse;
    }

    // If the swapchain recreation failes, exit before unsetting flag
    // This can happen when window is minimized
    if (!SwpRecreate()) {
      FWARN("VulkanBackend::BeginFrame(): SwpRecreate() failed");
      return FeFalse;
    }

    FINFO("VulkanBackend::BeginFrame(): Resize happenning");
    return FeFalse;
  }

  // Wait for the execution of the current frame to complete. The fence being
  // free will allow this one to move on
  if (!FenceWait(&_context.inFlightFences[_context.currentFrame], UINT64_MAX)) {
    FWARN("VulkanBackend::BeginFrame(): In-flight fence wait failed");
    return FeFalse;
  }

  constexpr VkFence FENCE = 0;

  // Acquire next image from the swapchain. Pass along the semaphore that should
  // signal when this completes. This same semaphore will later be waited on by
  // the queue submission to ensure this image is available
  if (!SwapchainAcquireNextImage(
          &_context.swapchain, UINT64_MAX,
          _context.imageAvailableSemaphores[_context.currentFrame], FENCE,
          &_context.imageIndex)) {
    return FeFalse;
  }

  // Begin recording commands
  CommandBuffer *cmdBuffer =
      &_context.graphicsCommandBuffers[_context.imageIndex];
  CommandBufferReset(cmdBuffer);
  CommandBufferBegin(cmdBuffer, FeFalse, FeFalse, FeFalse);

  // Prepare viewport and scissor
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = (float32)_context.framebufferHeight;
  viewport.width = (float32)_context.framebufferWidth;
  viewport.height = -viewport.y;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = _context.framebufferWidth;
  scissor.extent.height = _context.framebufferHeight;

  constexpr uint32 FIRST_VP = 0;
  constexpr uint32 VP_COUNT = 1;
  constexpr uint32 FIRST_SC = 0;
  constexpr uint32 SC_COUNT = 1;

  vkCmdSetViewport(cmdBuffer->handle, FIRST_VP, VP_COUNT, &viewport);
  vkCmdSetScissor(cmdBuffer->handle, FIRST_SC, SC_COUNT, &scissor);

  _context.mainRenderPass.width = _context.framebufferWidth;
  _context.mainRenderPass.height = _context.framebufferHeight;

  RenderPassBegin(cmdBuffer, &_context.mainRenderPass,
                  _context.swapchain.framebuffers[_context.imageIndex].handle);

  return FeTrue;
}

bool VulkanBackend::EndFrame(float32 deltaTime) {
  Device *device = &_context.device;
  CommandBuffer *cmdBuffer =
      &_context.graphicsCommandBuffers[_context.imageIndex];

  // End renderpass
  RenderPassEnd(cmdBuffer, &_context.mainRenderPass);

  // End command buffer
  CommandBufferEnd(cmdBuffer);

  // Make sure previous frame is not using this image (i.e. its fence is being
  // waited on)
  if (_context.imagesInFlight[_context.imageIndex] != VK_NULL_HANDLE) {
    FenceWait(&_context.inFlightFences[_context.currentFrame], UINT64_MAX);
  }

  // Mark the image as in use by this frame
  _context.imagesInFlight[_context.imageIndex] =
      &_context.inFlightFences[_context.currentFrame];

  // Reset the fence for use on the next frame
  FenceReset(&_context.inFlightFences[_context.currentFrame]);

  // Submit the queue and wait for the operation to complete
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer->handle;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores =
      &_context.imageAvailableSemaphores[_context.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &_context.queueCompleteSemaphores[_context.currentFrame];

  constexpr uint32 FLAG_COUNT = 1;
  constexpr uint32 SUBMIT_COUNT = 1;

  // Each semaphore waits on the corresponding pipeline stage to complete 1:1
  // ratio. VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent
  // color attachment writes from executing until the semaphore signals
  VkPipelineStageFlags flags[FLAG_COUNT] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = flags;

  VkResult result =
      vkQueueSubmit(device->graphicsQueue, SUBMIT_COUNT, &submitInfo,
                    _context.inFlightFences[_context.currentFrame].handle);
  if (result != VK_SUCCESS) {
    FERROR("VulkanBackend::EndFrame(): vkQueueSubmit failed: '%s'",
           utils::VkResultToString(result, FeTrue));
    return FeFalse;
  }

  CommandBufferUpdateSubmitted(cmdBuffer);

  // Give the image back to the swapchain
  SwapchainPresent(&_context.swapchain, device->graphicsQueue,
                   device->presentQueue,
                   _context.queueCompleteSemaphores[_context.currentFrame],
                   _context.imageIndex);

  return FeTrue;
}

/****** DEVICE LOGIC IMPLEMENTATION ******/

bool VulkanBackend::DeviceCreate() {
  if (!SelectPhysicalDevice()) {
    return FeFalse;
  }

  FINFO("VulkanBackend::DeviceCreate(): Creating logical device...");

  // Get queue family to not mess up the indices
  uint32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(_context.device.physicalDevice,
                                           &queueFamilyCount, nullptr);
  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(_context.device.physicalDevice,
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

  ADD_UNIQUE_QUEUE_INDEX(_context.device.graphicsQueueIndex);
  ADD_UNIQUE_QUEUE_INDEX(_context.device.presentQueueIndex);
  ADD_UNIQUE_QUEUE_INDEX(_context.device.transferQueueIndex);

  VkDeviceQueueCreateInfo queueCreateInfo[indexCount];
  for (uint32 i = 0; i < indexCount; i++) {
    queueCreateInfo[i].sType = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo[i].queueFamilyIndex = indices[i];

    uint32 maxQueues = queueFamilies[indices[i]].queueCount;

    queueCreateInfo[i].queueCount =
        (indices[i] == _context.device.graphicsQueueIndex && maxQueues >= 2)
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

  VK_CHECK(vkCreateDevice(_context.device.physicalDevice, &deviceCreateInfo,
                          _context.allocator, &_context.device.logicalDevice));

  FINFO("VulkanBackend::DeviceCreate(): Logical device created");

  // Queue selection
  vkGetDeviceQueue(_context.device.logicalDevice,
                   _context.device.graphicsQueueIndex, 0,
                   &_context.device.graphicsQueue);
  vkGetDeviceQueue(_context.device.logicalDevice,
                   _context.device.presentQueueIndex, 0,
                   &_context.device.presentQueue);
  vkGetDeviceQueue(_context.device.logicalDevice,
                   _context.device.transferQueueIndex, 0,
                   &_context.device.transferQueue);

  FINFO("VulkanBackend::DeviceCreate(): Queues obtained");

  VkCommandPoolCreateInfo poolCreateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolCreateInfo.queueFamilyIndex = _context.device.graphicsQueueIndex;
  poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK(vkCreateCommandPool(_context.device.logicalDevice, &poolCreateInfo,
                               _context.allocator,
                               &_context.device.graphicsCommandPool));

  FINFO("VulkanBackend::DeviceCreate(): Graphics command pool created");
  return FeTrue;
}

void VulkanBackend::DeviceDestroy() {
  _context.device.graphicsQueue = nullptr;
  _context.device.presentQueue = nullptr;
  _context.device.transferQueue = nullptr;

  FDEBUG("VulkanBackend::DeviceDestroy(): Destroying command pools...");
  vkDestroyCommandPool(_context.device.logicalDevice,
                       _context.device.graphicsCommandPool, _context.allocator);

  if (_context.device.logicalDevice) {
    FDEBUG("VulkanBackend::DeviceDestroy(): Destroying logical device...");
    vkDestroyDevice(_context.device.logicalDevice, _context.allocator);
    _context.device.logicalDevice = nullptr;
    FDEBUG("VulkanBackend::DeviceDestroy(): Logical device destroyed");
  }

  // Releasing resources on physical device
  FDEBUG(
      "VulkanBackend::DeviceDestroy(): Releasing physical device resources...");
  _context.device.physicalDevice = nullptr;

  if (_context.device.swapchainSupport.formats) {
    core::memory::MemoryManager::Free(
        _context.device.swapchainSupport.formats,
        sizeof(VkSurfaceFormatKHR) *
            _context.device.swapchainSupport.formatCount,
        core::memory::MEMORY_TAG_RENDERER);
    _context.device.swapchainSupport.formats = nullptr;
    _context.device.swapchainSupport.formatCount = 0;
  }

  if (_context.device.swapchainSupport.presentMode) {
    core::memory::MemoryManager::Free(
        _context.device.swapchainSupport.formats,
        sizeof(VkPresentModeKHR) *
            _context.device.swapchainSupport.presentModeCount,
        core::memory::MEMORY_TAG_RENDERER);
    _context.device.swapchainSupport.presentMode = nullptr;
    _context.device.swapchainSupport.presentModeCount = 0;
  }

  core::memory::MemoryManager::ZeroMemory(
      &_context.device.swapchainSupport.capabilities,
      sizeof(_context.device.swapchainSupport.capabilities));

  _context.device.graphicsQueueIndex = -1;
  _context.device.presentQueueIndex = -1;
  _context.device.transferQueueIndex = -1;
}

void VulkanBackend::QuerySwapchainSupport(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    SwapchainSupportInfo *outSwapchainInfo) {
  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &outSwapchainInfo->capabilities));

  // Surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &outSwapchainInfo->formatCount, nullptr));

  if (outSwapchainInfo->formatCount != 0) {
    if (!outSwapchainInfo->formats) {
      outSwapchainInfo->formats =
          (VkSurfaceFormatKHR *)core::memory::MemoryManager::Allocate(
              sizeof(VkSurfaceFormatKHR) * outSwapchainInfo->formatCount,
              core::memory::MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &outSwapchainInfo->formatCount,
        outSwapchainInfo->formats));
  }

  // Present mode
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &outSwapchainInfo->presentModeCount, nullptr));
  if (outSwapchainInfo->presentModeCount != 0) {
    if (!outSwapchainInfo->presentMode) {
      outSwapchainInfo->presentMode =
          (VkPresentModeKHR *)core::memory::MemoryManager::Allocate(
              sizeof(VkPresentModeKHR) * outSwapchainInfo->presentModeCount,
              core::memory::MEMORY_TAG_RENDERER);
    }

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &outSwapchainInfo->presentModeCount,
        outSwapchainInfo->presentMode));
  }
}

bool VulkanBackend::DeviceDetectDepthFormat(Device *device) {
  const uint64 candidateCount = 3;
  VkFormat candidates[3] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                            VK_FORMAT_D24_UNORM_S8_UINT};

  uint32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (uint64 i = 0; i < candidateCount; i++) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i],
                                        &props);

    if ((props.linearTilingFeatures & flags) == flags) {
      device->depthFormat = candidates[i];
      return FeTrue;
    } else if ((props.optimalTilingFeatures & flags) == flags) {
      device->depthFormat = candidates[i];
      return FeTrue;
    }
  }

  return FeFalse;
}

void VulkanBackend::SwapchainCreate(uint32 width, uint32 height,
                                    Swapchain *outSwapchain) {
  SwpCreate(width, height, outSwapchain);
}

void VulkanBackend::SwapchainRecreate(uint32 width, uint32 height,
                                      Swapchain *outSwapchain) {
  SwpDestroy(outSwapchain);
  SwpCreate(width, height, outSwapchain);
}

void VulkanBackend::SwapchainDestroy(Swapchain *swapchain) {
  SwpDestroy(swapchain);
}

bool VulkanBackend::SwapchainAcquireNextImage(
    Swapchain *swapchain, uint64 timeoutns, VkSemaphore imageAvailableSemaphore,
    VkFence fence, uint32 *outImageIndex) {
  VkResult result = vkAcquireNextImageKHR(
      _context.device.logicalDevice, swapchain->handle, timeoutns,
      imageAvailableSemaphore, fence, outImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // This errors happens when resizing goes wrong for example
    // We simply recreate the swapchain in this case
    SwapchainRecreate(_context.framebufferWidth, _context.framebufferHeight,
                      swapchain);
    return FeFalse;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    // These are only Vulkan errors and we simply log a fatal error
    // for the engine
    FFATAL("VulkanBackend::SwapchainAcquireNextImage(): Failed to acquire "
           "swapchain image");
    return FeFalse;
  }

  // If hits here, it means everything is ok
  return FeTrue;
}

void VulkanBackend::SwapchainPresent(Swapchain *swapchain,
                                     VkQueue graphicsQueue,
                                     VkQueue presentQueue,
                                     VkSemaphore renderCompleteSemaphore,
                                     uint32 presentImageIndex) {
  // Return the image to the swapchain for presentation
  VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pNext = nullptr;
  presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain->handle;
  presentInfo.pImageIndices = &presentImageIndex;
  presentInfo.pResults = nullptr;

  VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    // Must recreate
    SwapchainRecreate(_context.framebufferWidth, _context.framebufferHeight,
                      swapchain);
  } else if (result != VK_SUCCESS) {
    FFATAL(
        "VulkanBackend::SwapchainPresent(): Failed to present swapchain image");
  }

  // Increment and loop the index
  _context.currentFrame = (_context.currentFrame + 1) % swapchain->maxFrames;
}

/****** IMAGE LOGIC IMPLEMENTATION ******/

void VulkanBackend::ImageCreate(VkImageType image, uint32 width, uint32 height,
                                VkFormat format, VkImageTiling tiling,
                                VkImageUsageFlags usageFlags,
                                VkMemoryPropertyFlags memoryFlags,
                                bool createView, VkImageAspectFlags aspectFlags,
                                Image *outImage) {
  // Copy parameters to outImage
  outImage->width = width;
  outImage->height = height;

  VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.extent.width = width;
  imageCreateInfo.extent.height = height;
  /* TODO: MAKE THESE CONFIGURABLE */
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 4;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  /*********************************/
  imageCreateInfo.format = format;
  imageCreateInfo.tiling = tiling;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.usage = usageFlags;

  // Creates and bind the image
  VK_CHECK(vkCreateImage(_context.device.logicalDevice, &imageCreateInfo,
                         _context.allocator, &outImage->handle));

  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(_context.device.logicalDevice, outImage->handle,
                               &memoryRequirements);

  sint32 memoryType =
      _context.FindMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
  if (memoryType == -1) {
    FERROR("VulkanBackend::ImageCreate(): Required memory type not found. "
           "Invalid image");
  }

  VkMemoryAllocateInfo memoryAllocInfo = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memoryAllocInfo.allocationSize = memoryRequirements.size;
  memoryAllocInfo.memoryTypeIndex = memoryType;
  VK_CHECK(vkAllocateMemory(_context.device.logicalDevice, &memoryAllocInfo,
                            _context.allocator, &outImage->memory));
  VK_CHECK(vkBindImageMemory(_context.device.logicalDevice, outImage->handle,
                             outImage->memory, 0));

  if (createView) {
    outImage->view = nullptr;
    ImageViewCreate(format, outImage, aspectFlags);
  }
}

void VulkanBackend::ImageViewCreate(VkFormat format, Image *image,
                                    VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewCreateInfo = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCreateInfo.image = image->handle;
  /* TODO: Make this below configurable */
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  /**************************************/
  viewCreateInfo.format = format;
  viewCreateInfo.subresourceRange.aspectMask = aspectFlags;

  /* TODO: Make this below configurable */
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;
  /**************************************/

  VK_CHECK(vkCreateImageView(_context.device.logicalDevice, &viewCreateInfo,
                             _context.allocator, &image->view));

  FINFO("VulkanBackend::ImageViewCreate(): Image view created successfully");
}

void VulkanBackend::ImageDestroy(Image *image) {
  if (image->view) {
    vkDestroyImageView(_context.device.logicalDevice, image->view,
                       _context.allocator);
    image->view = nullptr;
  }

  if (image->memory) {
    vkFreeMemory(_context.device.logicalDevice, image->memory,
                 _context.allocator);
    image->memory = nullptr;
  }

  if (image->handle) {
    vkDestroyImage(_context.device.logicalDevice, image->handle,
                   _context.allocator);
    image->handle = nullptr;
  }
}

/****** RENDERPASS LOGIC IMPLEMENTATION ******/

void VulkanBackend::RenderPassCreate(RenderPass *outRenderPass, float32 x,
                                     float32 y, float32 width, float32 height,
                                     float32 r, float32 g, float32 b, float32 a,
                                     float32 depth, uint32 stencil) {
  // Main subpass struct
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  /***** TODO: MAKE ALL OF THIS CONFIGURABLE *****/
  uint32 attachmentDescriptionCount = 2;
  VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];

  // Color attachment
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = _context.swapchain.imageFormat.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  attachmentDescriptions[0] = colorAttachment;

  VkAttachmentReference colorAttachmentRef;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachmentRef.attachment = 0;

  // Subpass populate
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  // Depth attachment
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = _context.device.depthFormat;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  attachmentDescriptions[1] = depthAttachment;

  VkAttachmentReference depthAttachmentRef;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachmentRef.attachment = 1;

  // TODO: other attachments

  // Subpass populate
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  // Other attachments (explicitly null and 0 because not impl yet)
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;
  subpass.pPreserveAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;

  // Render pass dependencies
  VkSubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  // Render pass creation
  VkRenderPassCreateInfo renderPassCreateInfo = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassCreateInfo.attachmentCount = attachmentDescriptionCount;
  renderPassCreateInfo.pAttachments = attachmentDescriptions;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &dependency;
  renderPassCreateInfo.pNext = nullptr;
  renderPassCreateInfo.flags = 0;
  /***********************************************/

  VK_CHECK(vkCreateRenderPass(_context.device.logicalDevice,
                              &renderPassCreateInfo, _context.allocator,
                              &outRenderPass->handle));
}

void VulkanBackend::RenderPassDestroy(RenderPass *renderPass) {
  if (renderPass && renderPass->handle) {
    vkDestroyRenderPass(_context.device.logicalDevice, renderPass->handle,
                        _context.allocator);
    renderPass->handle = nullptr;
  }
}

void VulkanBackend::RenderPassBegin(CommandBuffer *cmdBuffer,
                                    RenderPass *renderPass,
                                    VkFramebuffer frameBuffer) {
  VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  beginInfo.renderPass = renderPass->handle;
  beginInfo.framebuffer = frameBuffer;
  beginInfo.renderArea.offset.x = renderPass->x;
  beginInfo.renderArea.offset.y = renderPass->y;
  beginInfo.renderArea.extent.width = renderPass->width;
  beginInfo.renderArea.extent.height = renderPass->height;

  /** TODO: Make these configurable **/
  const uint32 clearValCount = 2;
  VkClearValue clearVals[clearValCount];
  core::memory::MemoryManager::ZeroMemory(clearVals,
                                          sizeof(VkClearValue) * clearValCount);
  clearVals[0].color.float32[0] = renderPass->r;
  clearVals[0].color.float32[1] = renderPass->g;
  clearVals[0].color.float32[2] = renderPass->b;
  clearVals[0].color.float32[3] = renderPass->a;
  clearVals[1].depthStencil.depth = renderPass->depth;
  clearVals[1].depthStencil.stencil = renderPass->stencil;

  beginInfo.clearValueCount = clearValCount;
  beginInfo.pClearValues = clearVals;

  vkCmdBeginRenderPass(cmdBuffer->handle, &beginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  cmdBuffer->state = CMD_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanBackend::RenderPassEnd(CommandBuffer *cmdBuffer,
                                  RenderPass *renderPass) {
  vkCmdEndRenderPass(cmdBuffer->handle);
  cmdBuffer->state = CMD_BUFFER_STATE_RECORDING;
}

/****** COMMAND BUFFER IMPLEMENTATION ******/

void VulkanBackend::CommandBufferAllocate(VkCommandPool pool, bool isPrimary,
                                          CommandBuffer *outCmdBuffer) {
  // Zeros memory for this allocation
  core::memory::MemoryManager::ZeroMemory(outCmdBuffer, sizeof(CommandBuffer));

  VkCommandBufferAllocateInfo allocateInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandPool = pool;
  allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
                                 : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocateInfo.commandBufferCount = 1;
  allocateInfo.pNext = nullptr;

  outCmdBuffer->state = CMD_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(_context.device.logicalDevice,
                                    &allocateInfo, &outCmdBuffer->handle));
  outCmdBuffer->state = CMD_BUFFER_STATE_READY;
}

void VulkanBackend::CommandBufferFree(VkCommandPool pool,
                                      CommandBuffer *cmdBuffer) {
  constexpr uint32 CMD_BUFFER_COUNT = 1;
  vkFreeCommandBuffers(_context.device.logicalDevice, pool, CMD_BUFFER_COUNT,
                       &cmdBuffer->handle);
}

void VulkanBackend::CommandBufferBegin(CommandBuffer *cmdBuffer,
                                       bool isSingleUse,
                                       bool isRenderPassContinue,
                                       bool isSimultaneousUse) {
  VkCommandBufferBeginInfo beginInfo = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  beginInfo.flags = 0;
  if (isSingleUse) {
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }

  if (isRenderPassContinue) {
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  }

  if (isSimultaneousUse) {
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  }

  VK_CHECK(vkBeginCommandBuffer(cmdBuffer->handle, &beginInfo));
  cmdBuffer->state = CMD_BUFFER_STATE_RECORDING;
}

void VulkanBackend::CommandBufferEnd(CommandBuffer *cmdBuffer) {
  VK_CHECK(vkEndCommandBuffer(cmdBuffer->handle));
  cmdBuffer->state = CMD_BUFFER_STATE_RECORDING_ENDED;
}

void VulkanBackend::CommandBufferUpdateSubmitted(CommandBuffer *cmdBuffer) {
  cmdBuffer->state = CMD_BUFFER_STATE_SUBMITTED;
}

void VulkanBackend::CommandBufferReset(CommandBuffer *cmdBuffer) {
  cmdBuffer->state = CMD_BUFFER_STATE_READY;
}

void VulkanBackend::CommandBufferAllocateAndBeginSingleUse(
    VkCommandPool pool, CommandBuffer *cmdBuffer) {
  CommandBufferAllocate(pool, FeTrue, cmdBuffer);
  CommandBufferBegin(cmdBuffer, FeTrue, FeFalse, FeFalse);
}

void VulkanBackend::CommandBufferEndSingleUse(VkCommandPool pool,
                                              CommandBuffer *cmdBuffer,
                                              VkQueue queue) {
  CommandBufferEnd(cmdBuffer);

  // Submit the queue
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer->handle;

  constexpr uint32 SUBMIT_COUNT = 1;
  constexpr VkFence FENCE = 0;

  VK_CHECK(vkQueueSubmit(queue, SUBMIT_COUNT, &submitInfo, FENCE));
  VK_CHECK(vkQueueWaitIdle(queue));

  // Free the cmd buffer
  CommandBufferFree(pool, cmdBuffer);
}

/****** FRAMEBUFFER LOGIC IMPLEMENTATION ******/

void VulkanBackend::FrameBufferCreate(RenderPass *renderPass, uint32 width,
                                      uint32 height, uint32 attachmentCount,
                                      VkImageView *attachments,
                                      FrameBuffer *outFrameBuffer) {
  // Take a copy of the attachments, renderpass and attachment count
  outFrameBuffer->attachments =
      reinterpret_cast<VkImageView *>(core::memory::MemoryManager::Allocate(
          sizeof(VkImageView) * attachmentCount,
          core::memory::MEMORY_TAG_RENDERER));
  for (uint32 i = 0; i < attachmentCount; i++) {
    outFrameBuffer->attachments[i] = attachments[i];
  }
  outFrameBuffer->attachmentCount = attachmentCount;
  outFrameBuffer->renderPass = renderPass;

  constexpr uint32 LAYERS = 1;

  // Framebuffer create info
  VkFramebufferCreateInfo createInfo = {
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  createInfo.renderPass = renderPass->handle;
  createInfo.attachmentCount = attachmentCount;
  createInfo.pAttachments = outFrameBuffer->attachments;
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = LAYERS;

  VK_CHECK(vkCreateFramebuffer(_context.device.logicalDevice, &createInfo,
                               _context.allocator, &outFrameBuffer->handle));
}

void VulkanBackend::FrameBufferDestroy(FrameBuffer *frameBuffer) {
  vkDestroyFramebuffer(_context.device.logicalDevice, frameBuffer->handle,
                       _context.allocator);
  if (frameBuffer->attachments) {
    core::memory::MemoryManager::Free(frameBuffer->attachments,
                                      sizeof(VkImageView) *
                                          frameBuffer->attachmentCount,
                                      core::memory::MEMORY_TAG_RENDERER);
    frameBuffer->attachments = nullptr;
  }

  frameBuffer->handle = nullptr;
  frameBuffer->attachmentCount = 0;
  frameBuffer->renderPass = nullptr;
}

/****** FENCE LOGIC IMPLEMENTATION ******/

void VulkanBackend::FenceCreate(bool signaled, Fence *outFence) {
  outFence->isSignaled = signaled;
  VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  if (outFence->isSignaled) {
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  }

  VK_CHECK(vkCreateFence(_context.device.logicalDevice, &fenceCreateInfo,
                         _context.allocator, &outFence->handle));
}

void VulkanBackend::FenceDestroy(Fence *fence) {
  if (fence->handle) {
    vkDestroyFence(_context.device.logicalDevice, fence->handle,
                   _context.allocator);
    fence->handle = nullptr;
  }

  fence->isSignaled = FeFalse;
}

bool VulkanBackend::FenceWait(Fence *fence, uint64 timeoutns) {
  if (fence->isSignaled) {
    return FeTrue;
  }

  constexpr uint32 FENCE_COUNT = 1;
  constexpr bool WAIT_ALL = FeTrue;
  VkResult result = vkWaitForFences(_context.device.logicalDevice, FENCE_COUNT,
                                    &fence->handle, WAIT_ALL, timeoutns);

  switch (result) {
  case VK_SUCCESS:
    fence->isSignaled = FeTrue;
    return FeTrue;
  case VK_TIMEOUT:
    FWARN("VulkanBackend::FenceWait(): Fence wait timed out");
    break;
  case VK_ERROR_DEVICE_LOST:
    FERROR("VulkanBackend::FenceWait(): VK_ERROR_DEVICE_LOST found");
    break;
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    FERROR("VulkanBackend::FenceWait(): VK_ERROR_OUT_OF_HOST_MEMORY found");
    break;
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    FERROR("VulkanBackend::FenceWait(): VK_ERROR_OUT_OF_DEVICE_MEMORY found");
    break;
  default:
    FERROR("VulkanBackend::FenceWait(): Something unexpected happened");
    break;
  }

  return FeFalse;
}

void VulkanBackend::FenceReset(Fence *fence) {
  constexpr uint32 FENCE_COUNT = 1;
  if (fence->isSignaled) {
    VK_CHECK(vkResetFences(_context.device.logicalDevice, FENCE_COUNT,
                           &fence->handle));
    fence->isSignaled = FeFalse;
  }
}

/****** GETTERS & SETTERS ******/

void VulkanBackend::SetFrameBuffer(uint64 frameBuffer) {
  _frameBuffer = frameBuffer;
}

uint64 VulkanBackend::GetFrameBuffer() const { return _frameBuffer; }

const Context &VulkanBackend::GetContext() const { return _context; }

/****************************************************************/
/**************        PRIVATE MEHTODS         ******************/
/****************************************************************/

/****** SHUTDOWN LOGIC ******/

void VulkanBackend::Shutdown() {
  // Wait for logical device to finish
  vkDeviceWaitIdle(_context.device.logicalDevice);

  FDEBUG("VulkanBackend::Shutdown(): Destroying sync objects & fences...");
  for (uchar i = 0; i < _context.swapchain.maxFrames; i++) {
    if (_context.imageAvailableSemaphores[i]) {
      vkDestroySemaphore(_context.device.logicalDevice,
                         _context.imageAvailableSemaphores[i],
                         _context.allocator);

      _context.imageAvailableSemaphores[i] = nullptr;
    }

    if (_context.queueCompleteSemaphores[i]) {
      vkDestroySemaphore(_context.device.logicalDevice,
                         _context.queueCompleteSemaphores[i],
                         _context.allocator);

      _context.queueCompleteSemaphores[i] = nullptr;
    }

    FenceDestroy(&_context.inFlightFences[i]);
  }
  _context.imageAvailableSemaphores.Clear();
  _context.queueCompleteSemaphores.Clear();
  _context.inFlightFences.Clear();
  _context.imagesInFlight.Clear();

  FDEBUG("VulkanBackend::Shutdown(): Destroying command buffers...");
  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    if (_context.graphicsCommandBuffers[i].handle) {
      CommandBufferFree(_context.device.graphicsCommandPool,
                        &_context.graphicsCommandBuffers[i]);
      _context.graphicsCommandBuffers[i].handle = nullptr;
    }
  }
  _context.graphicsCommandBuffers.Clear();

  FDEBUG("VulkanBackend::Shutdown(): Destroying framebuffers...");
  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    FrameBufferDestroy(&_context.swapchain.framebuffers[i]);
  }
  _context.swapchain.framebuffers.Clear();

  FDEBUG("VulkanBackend::Shutdown(): Destroying render pass...");
  RenderPassDestroy(&_context.mainRenderPass);

  FDEBUG("VulkanBackend::Shutdown(): Destroying swapchain...");
  SwapchainDestroy(&_context.swapchain);

  FDEBUG("VulkanBackend::Shutdown(): Destroying/releasing device info...");
  DeviceDestroy();

  FDEBUG("VulkanBackend::Shutdown(): Destroying Vulkan debugger...");
  if (_context.debugMessenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            _context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(_context.instance, _context.debugMessenger, _context.allocator);
  }
}

/****** SWAPCHAIN LOGIC IMPLEMENTATION ******/

void VulkanBackend::SwpCreate(uint32 width, uint32 height,
                              Swapchain *outSwapchain) {
  VkExtent2D swapchainExtent = {width, height};
  outSwapchain->maxFrames = 2;

  bool found = FeFalse;
  for (uint32 i = 0; i < _context.device.swapchainSupport.formatCount; i++) {
    VkSurfaceFormatKHR format = _context.device.swapchainSupport.formats[i];
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
    outSwapchain->imageFormat = _context.device.swapchainSupport.formats[0];
  }

  // Set default as FIFO, but preferred is mailbox
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (uint32 i = 0; i < _context.device.swapchainSupport.presentModeCount;
       i++) {
    VkPresentModeKHR mode = _context.device.swapchainSupport.presentMode[i];
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = mode;
      break;
    }
  }

  // Requery swapchain support
  QuerySwapchainSupport(_context.device.physicalDevice, _context.surface,
                        &_context.device.swapchainSupport);

  // Swapchain support capabilities
  if (_context.device.swapchainSupport.capabilities.currentExtent.width !=
      UINT32_MAX) {
    swapchainExtent =
        _context.device.swapchainSupport.capabilities.currentExtent;
  }

  // Clamp to an allowed value by the GPU
  VkExtent2D min = _context.device.swapchainSupport.capabilities.minImageExtent;
  VkExtent2D max = _context.device.swapchainSupport.capabilities.maxImageExtent;
  swapchainExtent.width = FCLAMP(swapchainExtent.width, min.width, max.width);
  swapchainExtent.height =
      FCLAMP(swapchainExtent.height, min.height, max.height);

  uint32 imageCount =
      _context.device.swapchainSupport.capabilities.minImageCount + 1;
  if (_context.device.swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount >
          _context.device.swapchainSupport.capabilities.maxImageCount) {
    // Safegard clamp
    imageCount = _context.device.swapchainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchainCreateInfo.surface = _context.surface;
  swapchainCreateInfo.minImageCount = imageCount;
  swapchainCreateInfo.imageFormat = outSwapchain->imageFormat.format;
  swapchainCreateInfo.imageColorSpace = outSwapchain->imageFormat.colorSpace;
  swapchainCreateInfo.imageExtent = swapchainExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Setup the queue family indices
  if (_context.device.graphicsQueueIndex != _context.device.presentQueueIndex) {
    uint32 queueFamilyIndices[] = {(uint32)_context.device.graphicsQueueIndex,
                                   (uint32)_context.device.presentQueueIndex};
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  swapchainCreateInfo.preTransform =
      _context.device.swapchainSupport.capabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = presentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = nullptr;

  // VulkanBackend::SwapchainCreate the swapchain and store it
  VK_CHECK(vkCreateSwapchainKHR(_context.device.logicalDevice,
                                &swapchainCreateInfo, _context.allocator,
                                &outSwapchain->handle));

  // Starts with zero frames
  _context.currentFrame = 0;

  // Setting images
  outSwapchain->imageCount = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(_context.device.logicalDevice,
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
      _context.device.logicalDevice, outSwapchain->handle,
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
    VK_CHECK(vkCreateImageView(_context.device.logicalDevice, &viewInfo,
                               _context.allocator, &outSwapchain->views[i]));
  }

  // Depth features
  if (!DeviceDetectDepthFormat(&_context.device)) {
    _context.device.depthFormat = VK_FORMAT_UNDEFINED;
    FFATAL("VulkanBackend::SwapchainCreate(): Failed to find a supported depth "
           "format");
  }

  // Create depth image and its view
  ImageCreate(VK_IMAGE_TYPE_2D, swapchainExtent.width, swapchainExtent.height,
              _context.device.depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, FeTrue,
              VK_IMAGE_ASPECT_DEPTH_BIT, &outSwapchain->depthAttachment);

  FINFO("VulkanBackend::SwapchainCreate(): Image creation successful");
}

void VulkanBackend::SwpDestroy(Swapchain *swapchain) {
  ImageDestroy(&swapchain->depthAttachment);

  // Only destroys the views, not the image. Those are destroyed by the
  // swapchain
  for (uint32 i = 0; i < swapchain->imageCount; i++) {
    vkDestroyImageView(_context.device.logicalDevice, swapchain->views[i],
                       _context.allocator);
  }

  vkDestroySwapchainKHR(_context.device.logicalDevice, swapchain->handle,
                        _context.allocator);
}

bool VulkanBackend::SwpRecreate() {
  // If already recreating swapchain, don't try again
  if (_context.recreatingSwapchain) {
    FWARN("VulkanBackend::SwpRecreate(): called when already recreating");
    return FeFalse;
  }

  // Detect if the window is too small to be drawn to
  if (_context.framebufferWidth == 0 || _context.framebufferHeight == 0) {
    FWARN(
        "VulkanBackend::SwpRecreate(): called when window is < 1 in dimension");
    return FeFalse;
  }

  // Mark as recreating if above are valid sttmts
  // and wait idle
  _context.recreatingSwapchain = FeTrue;
  vkDeviceWaitIdle(_context.device.logicalDevice);

  // Clear images
  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    _context.imagesInFlight[i] = nullptr;
  }

  // Requery swapchain support
  QuerySwapchainSupport(_context.device.physicalDevice, _context.surface,
                        &_context.device.swapchainSupport);
  DeviceDetectDepthFormat(&_context.device);

  // Recreate the swapchain
  SwapchainRecreate(cachedFrameBufferWidth, cachedFrameBufferHeight,
                    &_context.swapchain);

  // Sync the framebuffer size with the cached sizes
  _context.framebufferWidth = cachedFrameBufferWidth;
  _context.framebufferHeight = cachedFrameBufferHeight;
  _context.mainRenderPass.width = _context.framebufferWidth;
  _context.mainRenderPass.height = _context.framebufferHeight;
  cachedFrameBufferWidth = cachedFrameBufferHeight = 0;

  // Updates framebuffer size gen
  _context.framebufferSizeLastGeneration = _context.framebufferSizeGeneration;

  // Clean swapchain and command buffers
  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    FrameBufferDestroy(&_context.swapchain.framebuffers[i]);
  }

  _context.mainRenderPass.x = 0;
  _context.mainRenderPass.y = 0;
  _context.mainRenderPass.width = _context.framebufferWidth;
  _context.mainRenderPass.height = _context.framebufferHeight;

  FrameBufferRegenerate(&_context.swapchain, &_context.mainRenderPass);
  CommandBuffersCreate();

  // Clear the recreating flag
  _context.recreatingSwapchain = FeFalse;
  return FeTrue;
}

/****** DEVICE LOGIC IMPLEMENTATION ******/

bool VulkanBackend::SelectPhysicalDevice() {
  uint32 physicalDeviceCount = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(_context.instance, &physicalDeviceCount,
                                      nullptr));
  if (physicalDeviceCount == 0) {
    FFATAL(
        "SelectPhysicalDevice(): No device which support Vulkan were found!");
    return FeFalse;
  }

  VkPhysicalDevice physicalDevices[physicalDeviceCount];
  VK_CHECK(vkEnumeratePhysicalDevices(_context.instance, &physicalDeviceCount,
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
        physicalDevices[i], _context.surface, &properties, &features,
        &requirements, &queueInfo, &_context.device.swapchainSupport);

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

    _context.device.physicalDevice = physicalDevices[i];
    _context.device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
    _context.device.presentQueueIndex = queueInfo.presentFamilyIndex;
    _context.device.transferQueueIndex = queueInfo.transferFamilyIndex;

    // Keep a copy of properties, features and memory for later use
    _context.device.properties = properties;
    _context.device.features = features;
    _context.device.memory = memory;
    break;
  }

  return FeTrue;
}

bool VulkanBackend::PhysicalDeviceMeetsRequirements(
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
      core::memory::unique_renderer_ptr<VkExtensionProperties> availableExts;
      VK_CHECK(vkEnumerateDeviceExtensionProperties(
          device, nullptr, &availableExtCount, nullptr));
      if (availableExtCount != 0) {
        availableExts =
            core::memory::unique_renderer_ptr<VkExtensionProperties>(
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

/****** COMMAND BUFFER LOGIC ******/
void VulkanBackend::CommandBuffersCreate() {
  if (_context.graphicsCommandBuffers.IsEmpty()) {
    _context.graphicsCommandBuffers.Reserve(_context.swapchain.imageCount);
    for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
      core::memory::MemoryManager::ZeroMemory(
          &_context.graphicsCommandBuffers[i], sizeof(CommandBuffer));
    }
  }

  for (uint32 i = 0; i < _context.swapchain.imageCount; i++) {
    if (_context.graphicsCommandBuffers[i].handle) {
      CommandBufferFree(_context.device.graphicsCommandPool,
                        &_context.graphicsCommandBuffers[i]);
    }
    core::memory::MemoryManager::ZeroMemory(&_context.graphicsCommandBuffers[i],
                                            sizeof(CommandBuffer));
    CommandBufferAllocate(_context.device.graphicsCommandPool, FeTrue,
                          &_context.graphicsCommandBuffers[i]);
  }
}

/****** FRAMEBUFFER LOGIC ******/
void VulkanBackend::FrameBufferRegenerate(Swapchain *swapchain,
                                          RenderPass *renderPass) {
  // TODO: remove constexpr
  constexpr uint32 ATTACHMENT_COUNT = 2;
  for (uint32 i = 0; i < swapchain->imageCount; i++) {
    // TODO: make this configurable based on attachments
    uint32 attachmentCount = ATTACHMENT_COUNT;
    VkImageView attachments[] = {swapchain->views[i],
                                 swapchain->depthAttachment.view};

    FrameBufferCreate(renderPass, _context.framebufferWidth,
                      _context.framebufferHeight, attachmentCount, attachments,
                      &_context.swapchain.framebuffers[i]);
  }
}

/***************** END VULKAN BACKEND CLASS IMPLEMENTATION ********************/

/****** VULKAN CALLBACKS ******/

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, uint32 messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
  switch (messageSeverity) {
  default:
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    FERROR(callbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    FWARN(callbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    FINFO(callbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    FTRACE(callbackData->pMessage);
    break;
  }

  // This is a must
  return VK_FALSE;
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
