#pragma once
#include "RenderInterfaceTypes.h"
#include "CommandListBase.h"
#include "SubmitBatch.h"
#include "SubmitDependency.h"
#include "VulkanRHI/Vulkan.h"
#include <array>
#include <initializer_list>

namespace toy::graphics::rhi::vulkan
{

	class VulkanCommandList : public detail::CommandListBase
	{
	public:
		VulkanCommandList(
			const QueueType queueType,
			RenderInterface& rhi) : CommandListBase(queueType, rhi){}
	protected:
		void barrierInternal(
			const std::initializer_list<BarrierDescriptor>& descriptors);

		void beginRenderingInternal(
			const RenderingDescriptor& descriptor,
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

		auto transferInternal(
			const SourceBufferDescriptor& srcBufferDescriptor,
			const DestinationImageDescriptor& dstImageDescription) -> void;

		auto bindPipelineInternal(const Handle<Pipeline>& pipeline) -> void;
		auto bindIndexBufferInternal(
			const Handle<Buffer>& buffer,
			const u64 offset,
			const IndexType indexType) -> void;

		auto setScissorInternal(const Scissor& scissor) -> void;
		auto setViewportInternal(const Viewport& viewport) -> void;

		auto bindGroupInternal(
			core::u32 set,
			const Handle<BindGroup>& handle) -> void;

		[[nodiscard]] auto buildAccelerationStructureInternal(
			const TriangleGeometry& geometry,
			const std::vector<AccelerationStructureDescriptor>& descriptors) -> std::vector<Handle<AccelerationStructure>>;

		[[nodiscard]] auto buildAccelerationStructureInternal(
			const std::initializer_list<AccelerationStructureInstance>& instances) -> Handle<AccelerationStructure>;

		auto beginInternal() -> void;
		auto endInternal() -> void;

		template <class ConstantValue>
		inline auto pushConstantInternal(const ConstantValue& value) -> void
		{
			commandBuffer_.pushConstants(currentPipeline_.layout, vk::ShaderStageFlagBits::eAll, 0,  sizeof(value), static_cast<const void*>(&value));
		}
		
	private:
		friend class VulkanRenderInterface;
		friend class VulkanRenderInterface;

		VulkanPipeline currentPipeline_{};
		vk::CommandBuffer commandBuffer_{}; 
	};
}