#pragma once
#include "VulkanRenderInterface.h"

namespace toy::renderer::api::vulkan
{
	class VulkanRenderInterface;

	class VulkanCommandList final : public CommandList
	{
	public:
		VulkanCommandList(const VulkanCommandList& other) = delete;
		VulkanCommandList(VulkanCommandList&& other) noexcept = default;
		VulkanCommandList& operator=(const VulkanCommandList& other) = default;
		VulkanCommandList& operator=(VulkanCommandList&& other) noexcept
		= default;

		explicit VulkanCommandList(VulkanRenderInterface& parent,
									vk::CommandBuffer commandBuffer,
		                           vk::CommandBufferLevel level,
		                           QueueType ownedQueueType);
		~VulkanCommandList() override;

		void barrierInternal(
			const std::initializer_list<BarrierDescriptor>&
			descriptors) override;

		void beginRenderingInternal(const RenderingDescriptor& descriptor,
		                            const RenderArea& area) override;
		void endRenderingInternal() override;
	protected:
		void drawInternal(u32 vertexCount,
		                  u32 instanceCount,
		                  u32 firstVertex,
		                  u32 firstInstance) override;
		void bindPipelineInternal(const Ref<Pipeline>& pipeline) override;
		void setScissorInternal(const Scissor& scissor) override;
		void setViewportInternal(const Viewport& viewport) override;

		void bindGroupInternal(u32 set, const Handle<BindGroup>& handle) override;
	private:
		friend VulkanRenderInterface;

		VulkanPipeline* currentPipeline_{};
		VulkanRenderInterface* renderInterface_;
		vk::CommandBuffer cmd_;
		vk::CommandBufferLevel level_;
		u32 index_{};
	};
}
