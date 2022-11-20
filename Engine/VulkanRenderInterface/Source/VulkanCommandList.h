#pragma once
#include <CommandList.h>
#include "Structs.h"
#include "Vulkan.h"


namespace toy::renderer::api::vulkan
{

	class VulkanRenderInterface;

	class VulkanCommandList final : public CommandList<VulkanCommandList, VulkanRenderInterface>
	{
	public:
		friend class RenderInterface<VulkanRenderInterface>;
		friend class CommandList<VulkanCommandList, VulkanRenderInterface>;
		friend class VulkanCommandList;
		friend class VulkanRenderInterface;
		using RenderInterfaceType = VulkanRenderInterface;

	
		explicit VulkanCommandList(const QueueType queueType, VulkanRenderInterface& renderInterface, const vk::CommandBuffer commandBuffer): CommandList(queueType, renderInterface), commandBuffer_(commandBuffer){}
	private:
		void barrierInternal(
			const std::initializer_list<BarrierDescriptor>& descriptors);

		void beginRenderingInternal(const RenderingDescriptor& descriptor,
			const RenderArea& area);
		void endRenderingInternal();

		void drawInternal(
			core::u32 vertexCount,
			core::u32 instanceCount,
			core::u32 firstVertex,
			core::u32 firstInstance);
		void bindPipelineInternal(const Handle<Pipeline>& pipeline);
		void setScissorInternal(const Scissor& scissor);
		void setViewportInternal(const Viewport& viewport);

		void bindGroupInternal(core::u32 set, const Handle<BindGroup>& handle);

		[[nodiscard]] auto buildAccelerationStructureInternal(
			const TriangleGeometry& geometry,
			const std::vector<AccelerationStructureDescriptor>& descriptors) -> std::vector<Handle<AccelerationStructure>>;
		[[nodiscard]] auto buildAccelerationStructureInternal(
			const std::initializer_list<AccelerationStructureInstance>& instances) -> Handle<AccelerationStructure>;
		void beginInternal();
		void endInternal();
		
		VulkanPipeline currentPipeline_{};
		vk::CommandBuffer commandBuffer_{}; 
	};
}