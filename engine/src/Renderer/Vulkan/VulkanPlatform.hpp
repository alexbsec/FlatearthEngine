#ifndef _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP
#define _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP

#include "Containers/DArray.hpp"
#include "Definitions.hpp"
#include "VulkanTypes.inl"

namespace flatearth {
namespace platform {

// Forward declaration
struct PlatformState;

bool CreateVulkanSurface(struct PlatformState *platState,
                         struct renderer::vulkan::Context *context);

void GetRequiredExtNames(containers::DArray<const char *> *namesDArray);

} // namespace platform
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_PLATFORM_HPP
