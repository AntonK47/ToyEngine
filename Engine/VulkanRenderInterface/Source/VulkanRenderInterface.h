#pragma once

#include <map>
#include <mutex>
#include <RenderInterface.h>
#include <memory>
#include <Hash.h>

#include "Structs.h"
#include "Vulkan.h"
#include "VulkanBindGroupAllocator.h"

class Application;

namespace toy::renderer::api::vulkan
{
	class VulkanCommandList;
	struct LinkedLinearAllocator;
	struct RingAllocator;

	template <typename T>
	struct LinearFrameAllocator
	{
	public:
		void nextFrame()
		{
			itemsCount = 0;
		}

		u32 allocate(const T item)
		{
			assert(itemsCount < maxStorage);
			storage[itemsCount] = item;
			itemsCount++;
			return itemsCount - 1;
		}
	private:
		constexpr static u32 maxStorage = 1000;

		std::array<T, maxStorage> storage;
		u32 itemsCount{};
	};

	template <typename HandleType, typename Value>
	class Pool final
	{
	public:

		Pool()
		{
			pool_.reserve(1000);//TODO: Do something smarter
		}

		struct EmptyKey{};

		template <typename Key = EmptyKey>
		Handle<HandleType> add(const Value& value, const Key& key = {})
		{
			struct
			{
				Key v;
				u32 uniqueValue;
			} a{key,uniqueValue_};

			uniqueValue_++;


			const auto hash = Hasher::hash32(a);
			pool_[hash] = value;

			return Handle<HandleType>{hash};
		}

		Value& get(const Handle<HandleType> handle)
		{
			return pool_[handle.index];
		}

		void remove(const Handle<HandleType> handle)
		{
			pool_.erase(handle.index);
		}

		void reset()
		{
			uniqueValue_ = 0;
			pool_.clear();
		}
	private:
		u32 uniqueValue_{};
		std::unordered_map<u32, Value> pool_{};
	};


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
		vk::PipelineLayout layout;
		vk::PipelineBindPoint bindPoint;
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



	private:
		

		std::unique_ptr<CommandList> acquireCommandListInternal(
			QueueType queueType,
			CommandListType commandListType) override;


		void initializeInternal(const RendererDescriptor& descriptor) override;
		void deinitializeInternal() override;
		void nextFrameInternal() override;

		[[nodiscard]] Handle<BindGroupLayout> allocateBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor) override;
		[[nodiscard]] Handle<BindGroup> allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout) override;
		[[nodiscard]] std::vector<Handle<BindGroup>> allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount) override;

	public:
		[[nodiscard]] SwapchainImage
		acquireNextSwapchainImageInternal() override;
		void presentInternal() override;
		void submitCommandListInternal(
			const std::unique_ptr<CommandList> commandList) override;

	protected:
		[[nodiscard]] std::unique_ptr<Pipeline> createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups) override;

		[[nodiscard]] std::unique_ptr<ShaderModule> createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) override;
	private:

		void resetDescriptorPoolsUntilFrame(const u32 frame);
	protected:
		[[nodiscard]] Handle<Buffer> createBufferInternal(
			const BufferDescriptor& descriptor) override;
	public:
		void updateBindGroupInternal(const Handle<BindGroup>& bindGroup,
			const std::initializer_list<BindingDataMapping>& mappings) override;
	protected:
		void mapInternal(Handle<Buffer> buffer, void** data) override;
	private:
		std::unordered_map<QueueType, DeviceQueue> queues_;

		friend VulkanCommandList;

		DeviceQueue presentQueue_{};
		vk::Semaphore readyToPresentSemaphore_;
		vk::Semaphore readyToRenderSemaphore_;
		u32 currentImageIndex_{};

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

		std::unordered_map<u32, vk::DescriptorSetLayout> bindGroupLayoutCache_{};

		LinearFrameAllocator<vk::DescriptorSet> bindGroupCache_{};

		std::array<std::vector<vk::DescriptorPool>, swapchainImagesCount_> descriptorPoolsPerFrame_;
		u32 lastFrameInUse_{};

		struct VulkanBuffer
		{
			vk::Buffer buffer;
			VmaAllocation allocation;
		};


		Pool<Buffer, VulkanBuffer> bufferStorage_{};
		Pool<BindGroup, VulkanBindGroup> bindGroupStorage_{};//TODO: this pool should be resetet each frame, it only contains the data for a current frame
	};
}
