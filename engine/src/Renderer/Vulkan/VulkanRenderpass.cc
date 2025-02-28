#include "VulkanRenderpass.hpp"
#include "Core/FeMemory.hpp"
#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

void CreateRenderPass(Context *context, RenderPass *outRenderPass, float32 x,
                      float32 y, float32 width, float32 height, float32 r,
                      float32 g, float32 b, float32 a, float32 depth,
                      uint32 stencil) {
  // Main subpass struct
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  /***** TODO: MAKE ALL OF THIS CONFIGURABLE *****/
  uint32 attachmentDescriptionCount = 2;
  VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];

  // Color attachment
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = context->swapchain.imageFormat.format;
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
  depthAttachment.format = context->device.depthFormat;
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

  VK_CHECK(vkCreateRenderPass(context->device.logicalDevice,
                              &renderPassCreateInfo, context->allocator,
                              &outRenderPass->handle));
}

void DestroyRenderPass(Context *context, RenderPass *renderPass) {
  if (renderPass && renderPass->handle) {
    vkDestroyRenderPass(context->device.logicalDevice, renderPass->handle,
                        context->allocator);
    renderPass->handle = nullptr;
  }
}

void BeginRenderPass(CommandBuffer *cmdBuffer, RenderPass *renderPass,
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

void EndRenderPass(CommandBuffer *cmdBuffer, RenderPass *renderPass) {
  vkCmdEndRenderPass(cmdBuffer->handle);
  cmdBuffer->state = CMD_BUFFER_STATE_RECORDING;
}

} // namespace vulkan
} // namespace renderer
} // namespace flatearth
