#include "VulkanBackend.hpp"
#include "Containers/DArray.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Vulkan/VulkanPlatform.hpp"
#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

static Context context;

bool InitializeVulkan(unique_backend_renderer_ptr &backend,
                      const char *applicationName,
                      struct platform::PlatformState *platState) {
  // TODO: custom allocator
  context.allocator = nullptr;

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

  VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));

  FINFO("InitializeVulkan(): Vulkan renderer successfully initialized");

  return FeTrue;
}

void ShutdownVulkan(unique_backend_renderer_ptr &backend) {}

void OnResizeVulkan(unique_backend_renderer_ptr &backend, ushort width,
                    ushort height) {}

bool BeginFrameVulkan(unique_backend_renderer_ptr &backend, float32 deltaTime) {
  return FeTrue;
}

bool EndFrameVulkan(unique_backend_renderer_ptr &backend, float32 deltaTime) {
  return FeTrue;
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
