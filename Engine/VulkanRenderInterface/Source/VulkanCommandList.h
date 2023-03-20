#pragma once
#include <CommandList.h>
#include "Structs.h"
#include "Vulkan.h"
#include <array>

namespace toy::renderer::api::vulkan
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

	class VulkanSubmitBatch final : public SubmitBatch<VulkanSubmitBatch>
	{
	public:
		friend class SubmitBatch<VulkanSubmitBatch>;
		friend class RenderInterface<VulkanRenderInterface>;
		friend class VulkanRenderInterface;
		explicit VulkanSubmitBatch(const Submit submit, const QueueType type) : SubmitBatch(type), batch_(submit){}

	private:
		[[nodiscard]] auto barrierInternal() -> SubmitDependency;

		Submit batch_;
	};

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

		auto drawIndexedInternal(
			core::u32 indexCount,
			core::u32 instanceCount,
			core::u32 firstIndex,
			core::i32 vertexOffset,
			core::u32 firstInstance
		) -> void;

		auto transferInternal(const SourceBufferDescriptor& srcBufferDescriptor, const DestinationImageDescriptor& dstImageDescription) -> void;

		void bindPipelineInternal(const Handle<Pipeline>& pipeline);
		auto bindIndexBufferInternal(const Handle<Buffer>& buffer, const u64 offset, const IndexType indexType) -> void;

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

		template <class ConstantValue>
		auto pushConstantInternal(const ConstantValue& value)
		{
			commandBuffer_.pushConstants(currentPipeline_.layout, vk::ShaderStageFlagBits::eAll, 0,  sizeof(value), static_cast<const void*>(&value));
		}
		
		VulkanPipeline currentPipeline_{};
		vk::CommandBuffer commandBuffer_{}; 
	};
}