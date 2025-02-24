#ifndef _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP
#define _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP

#include "VulkanTypes.inl"

namespace flatearth {
namespace renderer {
namespace vulkan {

bool CreateDevice(Context *context);

void DestroyDevice(Context *context);

}
}
}

#endif // _FLATEARTH_ENGINE_VULKAN_DEVICE_HPP
