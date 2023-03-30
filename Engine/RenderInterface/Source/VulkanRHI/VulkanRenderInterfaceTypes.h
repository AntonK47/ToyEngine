#pragma once
#include <Core.h>

#include <array>
#include <Hash.h>
#include <unordered_map>
#include <memory>
#include <memory_resource>

#include "VulkanRHI/Vulkan.h"

namespace toy::graphics::rhi::vulkan
{
	template <typename T>
	struct LinearFrameAllocator
	{
	public:
		void nextFrame()
		{
			itemsCount = 0;
		}

		core::u32 allocate(const T item)
		{
			assert(itemsCount < maxStorage);
			storage[itemsCount] = item;
			itemsCount++;
			return itemsCount - 1;
		}
	private:
		constexpr static core::u32 maxStorage = 1000;

		std::array<T, maxStorage> storage;
		core::u32 itemsCount{};
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

		void do_deallocate([[maybe_unused]] void* ptr, [[maybe_unused]] const size_t bytes, [[maybe_unused]] const size_t align) override {}

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

		struct EmptyKey {};

		template <typename Key = EmptyKey>
		Handle<HandleType> add(const Value& value, const Key& key = {})
		{
			struct
			{
				Key v;
				core::u32 uniqueValue;
			} a{ key,uniqueValue_ };

			uniqueValue_++;


			const auto hash = core::Hasher::hash32(a);

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
			return pool_.find(handle.index) != pool_.end();
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
		core::u32 uniqueValue_{};
		std::unordered_map<core::u32, Value> pool_{ 2000 }; //TODO: make atomic hash map
	};

	struct VulkanShaderModule final
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
		core::u32 cacheSize;
	};

	struct PipelineCache
	{
	private:
		bool hasInitialized{ false };
		vk::PipelineCache cache{};
		std::vector<core::u8> cacheDataStorage{};
		vk::Device device{};

	public:
		void initialize(const PipelineCacheDescriptor& descriptor)
		{
			cacheDataStorage.resize(descriptor.cacheSize);
			device = descriptor.device;

			const auto cacheCreateInfo = vk::PipelineCacheCreateInfo
			{
				.initialDataSize = static_cast<core::u32>(cacheDataStorage.size()),
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
	struct CommandLists
	{
		vk::CommandPool pool;
		std::vector<vk::CommandBuffer> commandBuffers;
		uint32_t currentCommandBufferIndex{};
		QueueType queueType{};
	};

	struct PerThreadVulkanData
	{
		std::vector<CommandLists> graphicListsPerFrame;
		//CommandLists asyncListsPerFrame;
		CommandLists uploadLists;
		//semaphore
	};

	

	struct SparseImageRequirements
	{
		vk::MemoryRequirements2 imageMemoryRequirements_;
		vk::SparseImageMemoryRequirements2 sparseImageMemoryRequirements_;
	};

	struct DeviceQueue
	{
		u32 familyIndex{};
		u32 queueIndex{};
		vk::Queue queue;
		float priority{ 1.0 };
	};

	struct RenderThread
	{
		RenderThread();

		//PerThreadVulkanData localData;
		u32 localDataIndex;

		inline static RenderThread* instance{ nullptr };
	};

	template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };

	struct VulkanBindGroupLayout
	{
		vk::DescriptorSetLayout layout;
		u32 lastBindVariableSize{ 0 };
	};

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
		vk::ImageAspectFlags aspect{};
	};

	/*struct VulkanBuffer
	{
		vk::Buffer buffer;
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};

		std::function<void(void)> deleter;
	};*/

	struct VulkanImageView
	{
		vk::ImageView imageView;
	};

	struct VulkanSampler
	{
		vk::Sampler sampler;
	};

	struct VulkanBindGroup
	{
		vk::DescriptorSet descriptorSet;
	};
}
