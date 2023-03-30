#pragma once

#include "SubmitBatchBase.h"
#include "SubmitDependency.h"
#include "VulkanRHI/Vulkan.h"
#include <array>

namespace toy::graphics::rhi::vulkan
{

	class VulkanRenderInterface;


	struct Submit
	{
		u64 waitGraphicsValue{};
		u64 waitAsyncComputeValue{};
		u64 waitTransferValue{};
		u32 commandBuffersCount{};
		std::array<vk::CommandBuffer, 10> commandBuffers{};
	};

	class VulkanSubmitBatch : public SubmitBatchBase
	{
	protected:
		friend VulkanRenderInterface;
		VulkanSubmitBatch(const QueueType type) : SubmitBatchBase(type){}
		[[nodiscard]] auto barrierInternal() -> SubmitDependency;

		Submit batch_;

		friend class VulkanRenderInterface;
	};
}