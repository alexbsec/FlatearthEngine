#include "VulkanBackend.hpp"
#include "Core/Logger.hpp"
#include "VulkanTypes.inl"

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
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;

  VkResult result =
      vkCreateInstance(&createInfo, context.allocator, &context.instance);
  if (result != VK_SUCCESS) {
    FERROR("InitializeVulkan(): vkCreateInstance() failed with result %u",
           result);
    return FeFalse;
  }

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
