#ifndef _FLATEARTH_ENGINE_VULKAN_IMAGE_HPP
#define _FLATEARTH_ENGINE_VULKAN_IMAGE_HPP

#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

void CreateImage(Context *context, VkImageType image, uint32 width,
                 uint32 height, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usageFlags,
                 VkMemoryPropertyFlags memoryFlags, bool createView,
                 VkImageAspectFlags aspectFlags, Image *outImage);

void CreateImageView(Context *context, VkFormat format, Image *image,
                     VkImageAspectFlags aspectFlags);

void DestroyImage(Context *context, Image *image);

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_IMAGE_HPP
