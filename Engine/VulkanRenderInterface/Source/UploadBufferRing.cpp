#include "UploadBufferRing.h"

#include <vk_mem_alloc.h>

using namespace toy::renderer::api::vulkan;

void UploadBufferRing::destroy()
{
	vmaUnmapMemory(vmaAllocator, buffer.allocation);
	vmaDestroyBuffer(vmaAllocator, buffer.buffer, buffer.allocation);
}

uint64_t UploadBufferRing::memoryAvailable() const
{
	if (end < start)
		return std::max(end, buffer.allocationInfo.size - start);
	return end - start;
}

void* UploadBufferRing::allocate(uint64_t size)
{
	const std::lock_guard lock(allocateMutex);
	assert(memoryAvailable() >= size);

	void* dataBegin;

	if (end < start)
	{
		if ((buffer.allocationInfo.size - start) >= size)
		{
			dataBegin = static_cast<void*>(static_cast<uint8_t*>(data) + start);

			start += size;
		}
		else
		{
			dataBegin = data;
			start = size;
		}
	}
	else
	{
		dataBegin = static_cast<void*>(static_cast<uint8_t*>(data) + start);
		start += size;
	}

	return dataBegin;

}

void UploadBufferRing::free(uint32_t size)
{
	const std::lock_guard lock(allocateMutex);
	assert(buffer.allocationInfo.size - memoryAvailable() >= size);

	if (end < start)
	{
		end += size;
	}
	else
	{
		if ((buffer.allocationInfo.size - end) >= size)
		{
			end += size;
		}
		else
		{
			end = size;
		}
	}
}

void UploadBufferRing::create(const VmaAllocator& allocator, uint64_t size)
{
	vmaAllocator = allocator;
	start = {};
	end = { size };
	auto bufferUsage = vk::BufferUsageFlags{};
	bufferUsage |= vk::BufferUsageFlagBits::eTransferDst;

	const auto bufferCreateInfo = vk::BufferCreateInfo
	{
		.size = size,
		.usage = bufferUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	const auto allocationCreateInfo = VmaAllocationCreateInfo
	{
		.flags = (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT),
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST
	};


	auto uploadBuffer = vk::Buffer{};
	auto uploadBufferAllocation = VmaAllocation{};
	auto uploadBufferAllocationInfo = VmaAllocationInfo{};
	auto result = vmaCreateBuffer(allocator,
	                              reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo),
	                              &allocationCreateInfo,
	                              reinterpret_cast<VkBuffer*>(&buffer),
	                              &uploadBufferAllocation, &uploadBufferAllocationInfo);

	/*
		error handling
	*/
	buffer.buffer = uploadBuffer;
	buffer.allocation = uploadBufferAllocation;
	buffer.allocationInfo = uploadBufferAllocationInfo;


	vmaMapMemory(allocator, buffer.allocation, &data);
}
