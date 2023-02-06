#pragma once

#include <map>
#include <mutex>
#include <RenderInterface.h>
#include <memory>
#include <memory_resource>
#include <Hash.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
//#include <folly/AtomicHashMap.h>
#include <unordered_map>


#include "VulkanBindGroupAllocator.h"
#include <rigtorp/MPMCQueue.h>
#include "VulkanCommandList.h"

class Application;

namespace toy::renderer
{
	template<>
	struct CommandListType<api::vulkan::VulkanRenderInterface>
	{
		using type = api::vulkan::VulkanCommandList;
	};

	template<>
	struct SubmitBatchType<api::vulkan::VulkanRenderInterface>
	{
		using type = api::vulkan::VulkanSubmitBatch;
	};
	
}

namespace toy::renderer::api::vulkan
{
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

	class LinearAllocator final : public std::pmr::memory_resource
	{
	public:
		explicit LinearAllocator(void* const buffer, const size_t size) noexcept
			: currentBufferPointer_(buffer), bufferBeginPointer_(buffer), spaceAvailable_(size), maxSize_(size)
		{}
		~LinearAllocator() noexcept override = default;

		void release()
		{
			currentBufferPointer_ = bufferBeginPointer_;
			spaceAvailable_ = maxSize_;
		}

		LinearAllocator(const LinearAllocator&) = delete;
		LinearAllocator& operator=(const LinearAllocator&) = delete;
	private:

		[[nodiscard]] bool do_is_equal(const memory_resource& other) const noexcept override
		{
			return this == &other;
		}

		void* do_allocate(const size_t bytes, const size_t align) override
		{
			void* result = std::align(align, bytes, currentBufferPointer_, spaceAvailable_);

			if (result == nullptr)
			{
				std::_Xbad_alloc();
			}

			currentBufferPointer_ = static_cast<char*>(result) + bytes;

			spaceAvailable_ = static_cast<char*>(bufferBeginPointer_) + maxSize_ - currentBufferPointer_;

			return result;
		}

		void do_deallocate([[maybe_unused]] void* ptr, [[maybe_unused]] const size_t bytes, [[maybe_unused]]const size_t align) override {}

		void* currentBufferPointer_ = nullptr;
		void* bufferBeginPointer_ = nullptr;
		size_t spaceAvailable_{};
		size_t maxSize_{};
	};

	template <typename HandleType, typename Value>
	class Pool final
	{
	public:

		explicit Pool() = default;

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

			auto ret = pool_.insert(std::make_pair(hash, value));
			
			//pool_[hash] = value;

			return Handle<HandleType>{hash};
		}

		[[nodiscard]] Value& get(const Handle<HandleType> handle)
		{
			return pool_.find(handle.index)->second;
		}

		bool contains(const Handle<HandleType> handle)
		{
			return pool_.find(handle.index)!=pool_.end();
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

		auto begin()
		{
			return pool_.begin();
		}

		auto end()
		{
			return pool_.end();
		}

	private:
		u32 uniqueValue_{};
		std::unordered_map<u32, Value> pool_{2000}; //TODO: make atomic hash map
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
		std::map<QueueType, std::vector<PerFrameCommandPoolData>> perQueueType;
	};

	

	struct PipelineCacheDescriptor
	{
		vk::Device device;
		u32 cacheSize;
	};

	struct PipelineCache
	{
	private:
		bool hasInitialized{ false };
		vk::PipelineCache cache{};
		std::vector<u8> cacheDataStorage{};
		vk::Device device{};

	public:
		void initialize(const PipelineCacheDescriptor& descriptor)
		{
			cacheDataStorage.resize(descriptor.cacheSize);
			device = descriptor.device;

			const auto cacheCreateInfo = vk::PipelineCacheCreateInfo
			{
				.initialDataSize = static_cast<u32>(cacheDataStorage.size()),
				.pInitialData = cacheDataStorage.data()

			};
			const auto result = device.createPipelineCache(cacheCreateInfo);
			TOY_ASSERT(result.result == vk::Result::eSuccess);
			cache = result.value;
			hasInitialized = true;
		}

		void deinitialize()
		{
			device.destroyPipelineCache(cache);
		}

		[[nodiscard]] vk::PipelineCache get() const
		{
			TOY_ASSERT(hasInitialized);
			return cache;
		}
	};


	
	

	class VulkanRenderInterface final : public RenderInterface<VulkanRenderInterface>
	{
		friend class RenderInterface<VulkanRenderInterface>;
		friend class VulkanCommandList;
	public:
		using CommandListType = VulkanCommandList;
		using SubmitBatchType = VulkanSubmitBatch;

	private:
		auto initializeInternal(const RendererDescriptor& descriptor) -> void;
		auto deinitializeInternal() -> void;


		[[nodiscard]] auto acquireCommandListInternal(
			QueueType queueType,
			const UsageScope& usageScope) -> CommandListType;

		auto submitCommandListInternal(
			const CommandListType& commandList) -> void;

		[[nodiscard]] auto submitCommandListInternal(
			QueueType queueType,
			const std::initializer_list<CommandListType>& commandLists,
			const std::initializer_list<SubmitDependency>& dependencies) -> SubmitBatchType;

		auto nextFrameInternal() -> void;

		[[nodiscard]] auto acquireNextSwapchainImageInternal() -> SwapchainImage;

		auto presentInternal() -> void;

		[[nodiscard]] auto getNativeBackendInternal() -> NativeBackend
		{
			return NativeBackend{ &nativeBackend_ };
		}


		

		[[nodiscard]] auto createBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor) -> Handle<BindGroupLayout>;

		[[nodiscard]] auto allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const UsageScope& scope) -> std::vector<Handle<BindGroup>>; //TODO: smallvector

		[[nodiscard]] auto createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) -> Handle<ShaderModule>;

		void resetDescriptorPoolsUntilFrame(const u32 frame);

		[[nodiscard]] auto createBufferInternal(
			const BufferDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label) -> Handle<Buffer>;

		auto updateBindGroupInternal(const Handle<BindGroup>& bindGroup,
		                             const std::initializer_list<
			                             BindingDataMapping>& mappings) -> void;

		auto mapInternal(const Handle<Buffer>& buffer, void** data) -> void;

		[[nodiscard]] auto createPipelineInternal(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createImageInternal(
			const ImageDescriptor& descriptor) -> Handle<Image>;
		[[nodiscard]] auto createImageViewInternal(
			const ImageViewDescriptor& descriptor) -> Handle<ImageView>;


		void submitBatchesInternal(const QueueType queueType, const std::initializer_list<SubmitBatchType>& batches);

		void initializePerRenderThreadData();


		std::unordered_map<QueueType, DeviceQueue> queues_;

		DeviceQueue presentQueue_{};
		vk::Semaphore readyToPresentSemaphore_;
		vk::Semaphore readyToRenderSemaphore_;
		u32 currentImageIndex_{};

		VmaAllocator allocator_{};

		VulkanNativeBackend nativeBackend_{};
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

		std::map<QueueType, vk::Semaphore> timelineSemaphorePerQueue_{};

		vk::SurfaceKHR surface_;
		vk::SwapchainKHR swapchain_;

		static constexpr u32 swapchainImagesCount_ = 3;
		std::vector<Handle<ImageView>> swapchainImageViews_{};
		std::vector<Handle<Image>> swapchainImages_{};
		std::vector<vk::Fence> swapchainImageAfterPresentFences_{};

		struct VulkanBindGroupLayout
		{
			vk::DescriptorSetLayout layout;
			u32 lastBindVariableSize{ 0 };
		};

		std::unordered_map<u32, VulkanBindGroupLayout> bindGroupLayoutCache_{};

		LinearFrameAllocator<vk::DescriptorSet> bindGroupCache_{};

		std::array<std::vector<vk::DescriptorPool>, swapchainImagesCount_> descriptorPoolsPerFrame_;

		std::vector<vk::DescriptorPool> descriptorPoolsPersistent_;
		u32 lastFrameInUse_{};

		struct VulkanBuffer
		{
			vk::Buffer buffer;
			VmaAllocation allocation{};
			bool isMapped{ false };
		};

		struct VulkanImage
		{
			vk::Image image;
			VmaAllocation allocation{};
			bool isMapped{ false };
			bool isExternal{ false };
		};

		struct VulkanImageView
		{
			vk::ImageView imageView;
		};


		static constexpr u32 maxCommandListsPerSubmit_ = 10;
		static constexpr u32 maxSubmits_ = 100;


		PipelineCache graphicsPipelineCache_{};
		PipelineCache computePipelineCache_{};

		Pool<ShaderModule, VulkanShaderModule> shaderModuleStorage_{};
		Pool<Pipeline, VulkanPipeline> pipelineStorage_{};

		Pool<Buffer, VulkanBuffer> bufferStorage_{};
		Pool<Image, VulkanImage> imageStorage_{};

		Pool<ImageView, VulkanImageView> imageViewStorage_{};

		Pool<BindGroup, VulkanBindGroup> bindGroupStorage_{};
		Pool<BindGroup, VulkanBindGroup> persistentBindGroupStorage_{};//TODO:: bind groups should be removed manual
	
		

		/*
		 *
		 *I need two independent frame allocators, one on the Host site and other living on device
		 *
		 */


	};

	
}
