#pragma once

#include "VulkanRHI/VulkanSubmitBatch.h"
#include "SubmitDependency.h"

namespace toy::graphics::rhi
{
	class SubmitBatch final
#ifdef TOY_ENGINE_VULKAN_BACKEND
		: public vulkan::VulkanSubmitBatch
#endif
	{
	public:
		SubmitBatch(const QueueType type) : VulkanSubmitBatch(type){}
		SubmitBatch() : VulkanSubmitBatch(QueueType::graphics) {};
		[[nodiscard]] auto barrier() -> SubmitDependency
		{
			return barrierInternal();
		}
	};
}