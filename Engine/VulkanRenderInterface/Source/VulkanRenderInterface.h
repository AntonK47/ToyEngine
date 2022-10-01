#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <map>
#include "Vulkan.h"
#include <vk_mem_alloc.h>
#include "RenderInterface.h"
#include <mutex>
#include "Structs.h"
#include "UploadBufferRing.h"

class Application;

namespace toy::renderer::api::vulkan
{

#ifdef VALIDATION
#define IF_VALIDATION_ENABLED(statement) statement
#else
#define IF_VALIDATION_ENABLED(statement)
#endif

	class VulkanRenderInterface;


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

	class VulkanRenderInterface : public RenderInterface
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
		Handle<renderer::Buffer> createBuffer(const BufferDescriptor& descriptor) override
		{
			return Handle<renderer::Buffer>{};
		}
		std::unique_ptr<CommandList> acquireCommandList(QueueType queueType, CommandListType commandListType) override;
		

		void initialize(RendererDescriptor descriptor) override;
		void deinitialize() override;
		void nextFrame() override;
		Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) override;
		Handle<Pipeline> createPipeline(const GraphicsPipelineDescriptor& graphicsPipelineDescription,
			const std::vector<BindGroup>& bindGroups) override;

	private:
		std::unordered_map<QueueType, DeviceQueue> queues_;

		DeviceQueue presentQueue_;

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
	};
}
