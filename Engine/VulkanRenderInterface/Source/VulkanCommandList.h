#pragma once

#include "VulkanRenderInterface.h"

using namespace toy::renderer;


namespace toy::renderer::api::vulkan
{
	class VulkanRenderInterface;

	class VulkanCommandList final: public CommandList
	{
	public:
		void barrier(const std::initializer_list<BarrierDescriptor>& descriptions) override
		{

		}
		Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& description) override
		{
			return Handle<SplitBarrier>{};
		}
		void endSplitBarrier(Handle<SplitBarrier> barrier) override
		{

		}
		~VulkanCommandList() override
		{

		}

		VulkanCommandList(vk::CommandBuffer commandBuffer, vk::CommandBufferLevel level):cmd_(commandBuffer), level_(level)
		{
			
		}

		void beginRendering(RenderingDescriptor description) override;

		void endRendering() override;
	private:
		friend VulkanRenderInterface;

		

		vk::CommandBuffer cmd_;
		vk::CommandBufferLevel level_;
		u32 index_{};
	};
}
