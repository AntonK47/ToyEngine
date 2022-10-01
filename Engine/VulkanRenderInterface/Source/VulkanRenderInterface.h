#pragma once

#include <map>
#include <mutex>
#include <RenderInterface.h>

#include "Structs.h"
#include "UploadBufferRing.h"
#include "Vulkan.h"
#include "VulkanBindGroupAllocator.h"

class Application;

namespace toy::renderer::api::vulkan
{

#ifdef VALIDATION
#define IF_VALIDATION_ENABLED(statement) statement
#else
#define IF_VALIDATION_ENABLED(statement)
#endif

	class VulkanRenderInterface;


	struct VulkanImage final: ImageResource
	{
		vk::Image image;
	};

	struct VulkanImageView final: ImageView
	{
		vk::ImageView vulkanImageView;
	};


	struct PerFrameCommandPoolData
	{
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
	};

	struct PerThreadCommandPoolData
	{
		vk::CommandBufferLevel level;
		std::map<QueueType, std::vector<PerFrameCommandPoolData>> perQueueType;
	};

	struct UploadBufferRing;

	class VulkanRenderInterface final: public RenderInterface
	{
	public:

		VulkanRenderInterface();
		virtual ~VulkanRenderInterface();

		
		void upload();
		void upload(const Handle<Buffer>& dstBuffer, uint64_t dstOffset, void* data, uint64_t size);

		UploadBufferRing& getUploadBufferRing() { return uploadBuffer_; }


	private:
		template <typename T>
		Handle<T> insertResource(const T& resource)
		{
			if (typeid(T) == typeid(toy::renderer::api::vulkan::VulkanBuffer))
			{
				return bufferPool_.insert(resource);
			}
			return bufferPool_.insert(resource);
		}

	public:
		std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType) override;
		

		void initialize(RendererDescriptor descriptor) override;
		void deinitialize() override;
		void nextFrame() override;

	private:
		[[nodiscard]] BindGroupLayout allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor) override;
	public:
		[[nodiscard]] SwapchainImage acquireNextSwapchainImage() override;
		void present() override;
		void submitCommandList(const std::unique_ptr<CommandList> commandList) override;

	private:
		std::unordered_map<QueueType, DeviceQueue> queues_;

		DeviceQueue presentQueue_;
		vk::Semaphore readyToPresentSemaphore_;
		vk::Semaphore readyToRenderSemaphore_;
		u32 currentImageIndex_{};

		BufferPool bufferPool_;
		VmaAllocator allocator_{};

		vk::Device device_;
		vk::Instance instance_;
		vk::PhysicalDevice adapter_;

		UploadBufferRing uploadBuffer_;

		const u32 maxCommandListsPerFrame_{ 10 };
		const u32 maxDeferredFrames_{ 3 };


		u32 currentFrame_;
		std::thread::id renderThreadId_;
		PerThreadCommandPoolData renderThreadCommandPoolData_{};
		


		std::vector<vk::Semaphore> timelineSemaphorePerFrame_{};

		vk::SurfaceKHR surface_;
		vk::SwapchainKHR swapchain_;

		const u32 swapchainImagesCount_ = 3;
		std::vector<vk::ImageView> swapchainImageViews_;
		std::vector<vk::Image> swapchainImages_;
		std::vector<vk::Fence> swapchainImageAfterPresentFences_;
	};
}
