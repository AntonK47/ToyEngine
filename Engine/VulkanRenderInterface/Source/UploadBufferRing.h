#pragma once
#include <mutex>

#include "Structs.h"
#include "Vulkan.h"

namespace toy::renderer::api::vulkan
{
	struct UploadBufferRing
	{
		uint64_t start{};
		uint64_t end{};
		VulkanBuffer buffer{};
		void* data{};
		VmaAllocator vmaAllocator{};
		mutable std::mutex allocateMutex{};

		void create(const VmaAllocator& allocator, uint64_t size);
		void destroy();
		uint64_t memoryAvailable() const;
		void* allocate(uint64_t size);
		void free(uint32_t size);
	};
}
