#pragma once
#include "VulkanRenderInterface.h"

namespace toy::renderer::api::vulkan
{
	class VulkanRenderInterface;

	class VulkanCommandList final: public CommandList
	{
	public:
		VulkanCommandList(const VulkanCommandList& other) = delete;
		VulkanCommandList(VulkanCommandList&& other) noexcept = default;
		VulkanCommandList& operator=(const VulkanCommandList& other) = default;
		VulkanCommandList& operator=(VulkanCommandList&& other) noexcept = default;

		explicit VulkanCommandList(vk::CommandBuffer commandBuffer, vk::CommandBufferLevel level, QueueType ownedQueueType);
		~VulkanCommandList() override;

		void barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors) override;

		void beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area)override;
		void endRenderingInternal() override;

	private:
		friend VulkanRenderInterface;

		vk::CommandBuffer cmd_;
		vk::CommandBufferLevel level_;
		u32 index_{};
	};
}
