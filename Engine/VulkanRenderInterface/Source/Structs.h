#pragma once

#include <RenderInterface.h>
#include <vk_mem_alloc.h>

#include "Vulkan.h"


namespace toy::renderer::api::vulkan
{
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

	struct VulkanBuffer : renderer::Buffer
	{
		vk::Buffer buffer;
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};

		std::function<void(void)> deleter;
	};

	/*struct ImageBuffer : renderer::Buffer
	{
		vk::Image image;
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};

		std::function<void(void)> deleter;
	};*/

	//template <typename T>
	//struct Pool
	//{
	//	Handle<T> insert(const T& item)
	//	{
	//		const auto index = static_cast<uint32_t>(pool.size());
	//		auto handler = Handle<T>{};
	//		handler.index = index;
	//		pool.push_back(item);
	//		return handler;
	//	}

	//	inline void clear()
	//	{
	//		for(auto& item : pool)
	//		{
	//			//item.deleter();
	//		}
	//		pool.clear();
	//	}
	//private:
	//	std::vector<T> pool{};
	//};

	//using BufferPool = Pool<Buffer>;

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
}
