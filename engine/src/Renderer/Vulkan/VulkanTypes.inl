#ifndef _FLATEARTH_ENGINE_VULKAN_TYPES_INL
#define _FLATEARTH_ENGINE_VULKAN_TYPES_INL

#include "Core/Asserts.hpp"
#include "Definitions.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

#define VK_CHECK(expr)                                                         \
  {                                                                            \
    FASSERT(expr == VK_SUCCESS)                                                \
  }

struct Context {
  VkInstance instance;
  VkAllocationCallbacks *allocator;
#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_TYPES_INL
