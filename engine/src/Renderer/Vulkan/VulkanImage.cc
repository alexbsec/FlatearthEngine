#include "VulkanImage.hpp"
#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

void CreateImage(Context *context, VkImageType image, uint32 width,
                 uint32 height, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usageFlags,
                 VkMemoryPropertyFlags memoryFlags, bool createView,
                 VkImageAspectFlags aspectFlags, Image *outImage) {
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
  VK_CHECK(vkCreateImage(context->device.logicalDevice, &imageCreateInfo,
                         context->allocator, &outImage->handle));

  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(context->device.logicalDevice, outImage->handle,
                               &memoryRequirements);

  sint32 memoryType =
      context->FindMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
  if (memoryType == -1) {
    FERROR("CreateImage(): Required memory type not found. Invalid image");
  }

  VkMemoryAllocateInfo memoryAllocInfo = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memoryAllocInfo.allocationSize = memoryRequirements.size;
  memoryAllocInfo.memoryTypeIndex = memoryType;
  VK_CHECK(vkAllocateMemory(context->device.logicalDevice, &memoryAllocInfo,
                            context->allocator, &outImage->memory));
  VK_CHECK(vkBindImageMemory(context->device.logicalDevice, outImage->handle,
                             outImage->memory, 0));

  if (createView) {
    outImage->view = nullptr;
    CreateImageView(context, format, outImage, aspectFlags);
  }
}

void CreateImageView(Context *context, VkFormat format, Image *image,
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

  VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewCreateInfo,
                             context->allocator, &image->view));

  FINFO("CreateImageView(): Image view created successfully");
}

void DestroyImage(Context *context, Image *image) {
  if (image->view) {
    vkDestroyImageView(context->device.logicalDevice, image->view,
                       context->allocator);
    image->view = nullptr;
  }

  if (image->memory) {
    vkFreeMemory(context->device.logicalDevice, image->memory,
                 context->allocator);
    image->memory = nullptr;
  }

  if (image->handle) {
    vkDestroyImage(context->device.logicalDevice, image->handle,
                   context->allocator);
    image->handle = nullptr;
  }
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
