#ifndef _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
#define _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP

#include "Containers/DArray.hpp"
#include "Renderer/RendererTypes.inl"
#include "VulkanTypes.inl"
#include <vulkan/vulkan_core.h>

namespace flatearth {
namespace renderer {
namespace vulkan {

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

class VulkanBackend : public IRendererBackend {
public:
  ~VulkanBackend();

  bool Initialize(const char *applicationName,
                  struct platform::PlatformState *platState) override;
  void OnResize(ushort width, ushort height) override;
  bool BeginFrame(float32 deltaTime) override;
  bool EndFrame(float32 deltaTime) override;

  // Device related logic
  bool DeviceCreate();
  void DeviceDestroy();
  void QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface,
                             SwapchainSupportInfo *outSwapchainInfo);
  bool DeviceDetectDepthFormat(Device *device);

  // Swapchain related logic
  void SwapchainCreate(uint32 width, uint32 height, Swapchain *outSwapchain);
  void SwapchainRecreate(uint32 width, uint32 height, Swapchain *outSwapchain);
  void SwapchainDestroy(Swapchain *swapchain);
  bool SwapchainAcquireNextImage(Swapchain *swapchain, uint64 timeoutns,
                                 VkSemaphore imageAvailableSemaphore,
                                 VkFence fence, uint32 *outImageIndex);
  void SwapchainPresent(Swapchain *swapchain, VkQueue graphicsQueue,
                        VkQueue presentQueue,
                        VkSemaphore renderCompleteSemaphore,
                        uint32 presentImageIndex);

  // Image related logic
  void ImageCreate(VkImageType image, uint32 width, uint32 height,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usageFlags,
                   VkMemoryPropertyFlags memoryFlags, bool createView,
                   VkImageAspectFlags aspectFlags, Image *outImage);
  void ImageViewCreate(VkFormat format, Image *image,
                       VkImageAspectFlags aspectFlags);
  void ImageDestroy(Image *image);

  // Render pass logic
  void RenderPassCreate(RenderPass *outRenderPass, float32 x, float32 y,
                        float32 width, float32 height, float32 r, float32 g,
                        float32 b, float32 a, float32 depth, uint32 stencil);
  void RenderPassDestroy(RenderPass *renderPass);
  void RenderPassBegin(CommandBuffer *cmdBuffer, RenderPass *renderPass,
                       VkFramebuffer frameBuffer);
  void RenderPassEnd(CommandBuffer *cmdBuffer, RenderPass *renderPass);

  // Command buffer logic
  void CommandBufferAllocate(VkCommandPool pool, bool isPrimary,
                             CommandBuffer *outCmdBuffer);
  void CommandBufferFree(VkCommandPool pool, CommandBuffer *cmdBuffer);
  void CommandBufferBegin(CommandBuffer *cmdBuffer, bool isSingleUse,
                          bool isRenderPassContinue, bool isSimultaneousUse);
  void CommandBufferEnd(CommandBuffer *cmdBuffer);
  void CommandBufferUpdateSubmitted(CommandBuffer *cmdBuffer);
  void CommandBufferReset(CommandBuffer *cmdBuffer);
  void CommandBufferAllocateAndBeginSingleUse(VkCommandPool pool,
                                              CommandBuffer *outCmdBuffer);
  void CommandBufferEndSingleUse(VkCommandPool pool, CommandBuffer *cmdBuffer,
                                 VkQueue queue);

  // Framebuffer logic
  void FrameBufferCreate(RenderPass *renderPass, uint32 width, uint32 height,
                         uint32 attachmentCount, VkImageView *attachments,
                         FrameBuffer *outFrameBuffer);
  void FrameBufferDestroy(FrameBuffer *frameBuffer);

  // Setters & getters
  void SetFrameBuffer(uint64 frameBuffer) override;
  uint64 GetFrameBuffer() const override;
  const Context &GetContext() const;

private:
  void Shutdown();

  void SwpCreate(uint32 width, uint32 height, Swapchain *outSwapchain);
  void SwpDestroy(Swapchain *swapchain);

  bool SelectPhysicalDevice();
  bool PhysicalDeviceMeetsRequirements(
      VkPhysicalDevice device, VkSurfaceKHR surface,
      const VkPhysicalDeviceProperties *properties,
      const VkPhysicalDeviceFeatures *features,
      const PhysicalDeviceRequirements *requirements,
      PhysicalDeviceQueueFamilyInfo *outQueueInfo,
      SwapchainSupportInfo *outSwapchainInfo);

  void CommandBuffersCreate();

  Context _context;
  struct platform::PlatformState *_platState;
  uint64 _frameBuffer;
};

} // namespace vulkan
} // namespace renderer
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_VULKAN_BACKEND_HPP
