#pragma once

#include <map>
#include <mutex>
#include <RenderInterface.h>

#include "Structs.h"
#include "Vulkan.h"
#include "VulkanBindGroupAllocator.h"

class Application;

namespace toy::renderer::api::vulkan
{
	class VulkanRenderInterface;

	struct VulkanImage final : ImageResource
	{
		vk::Image image;
	};

	struct VulkanImageView final : ImageView
	{
		vk::ImageView vulkanImageView;
	};

	struct VulkanShaderModule final : ShaderModule
	{
		vk::ShaderModule module;
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


	struct VulkanPipeline final : Pipeline
	{
		vk::Pipeline pipeline;
	};

	struct UploadBufferRing;

	class VulkanRenderInterface final : public RenderInterface
	{
	public:
		VulkanRenderInterface(const VulkanRenderInterface& other) = delete;
		VulkanRenderInterface(VulkanRenderInterface&& other) noexcept = default;
		VulkanRenderInterface& operator=(const VulkanRenderInterface& other)
		= default;
		VulkanRenderInterface& operator=(VulkanRenderInterface&& other) noexcept
		= default;

		explicit VulkanRenderInterface() = default;
		~VulkanRenderInterface() override;


		void upload();
		void upload(const Handle<Buffer>& dstBuffer,
		            uint64_t dstOffset,
		            void* data,
		            uint64_t size);

		//UploadBufferRing& getUploadBufferRing() { return uploadBuffer_; }


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

		std::unique_ptr<CommandList> acquireCommandListInternal(
			QueueType queueType,
			CommandListType commandListType) override;


		void initializeInternal(const RendererDescriptor& descriptor) override;
		void deinitializeInternal() override;
		void nextFrameInternal() override;

		[[nodiscard]] BindGroupLayout allocateBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor) override;

	public:
		[[nodiscard]] SwapchainImage
		acquireNextSwapchainImageInternal() override;
		void presentInternal() override;
		void submitCommandListInternal(
			const std::unique_ptr<CommandList> commandList) override;

	protected:
		[[nodiscard]] std::unique_ptr<Pipeline> createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<BindGroupDescriptor>& bindGroups) override;

		[[nodiscard]] std::unique_ptr<ShaderModule> createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) override;
	private:
		std::unordered_map<QueueType, DeviceQueue> queues_;

		DeviceQueue presentQueue_{};
		vk::Semaphore readyToPresentSemaphore_;
		vk::Semaphore readyToRenderSemaphore_;
		u32 currentImageIndex_{};

		BufferPool bufferPool_{};
		VmaAllocator allocator_{};

		vk::Device device_;
		vk::Instance instance_;
		vk::PhysicalDevice adapter_;

		//UploadBufferRing uploadBuffer_{};

		static constexpr u32 maxCommandListsPerFrame_{10};
		static constexpr u32 maxDeferredFrames_{3};


		u32 currentFrame_{};
		std::thread::id renderThreadId_;
		PerThreadCommandPoolData renderThreadCommandPoolData_{};


		std::vector<vk::Semaphore> timelineSemaphorePerFrame_{};

		vk::SurfaceKHR surface_;
		vk::SwapchainKHR swapchain_;

		static constexpr u32 swapchainImagesCount_ = 3;
		std::vector<vk::ImageView> swapchainImageViews_{};
		std::vector<vk::Image> swapchainImages_{};
		std::vector<vk::Fence> swapchainImageAfterPresentFences_{};
	};
}
