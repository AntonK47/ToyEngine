#pragma once
#include <vector>
#include <glm/ext/matrix_common.hpp>
#include <glm/glm.hpp>

#include "CommandListValidator.h"
#include "RenderInterfaceTypes.h"
#include "ValidationCommon.h"
#include <initializer_list>

#include "VulkanRHI/VulkanCommandList.h"

namespace toy::graphics::rhi
{
	class RenderInterface;
}

namespace toy::graphics::rhi
{
	struct BindGroup;
	struct ImageResource;
	enum class QueueType;
	

	class CommandList final
#ifdef TOY_ENGINE_VULKAN_BACKEND
		: public vulkan::VulkanCommandList
#endif
	{
	public:
		CommandList(const CommandList& other) = default;
		CommandList(CommandList&& other) noexcept = default;
		CommandList& operator=(const CommandList& other) = default;
		CommandList& operator=(CommandList&& other) noexcept = default;

		CommandList(const QueueType queueType, RenderInterface& renderInterface)
#ifdef TOY_ENGINE_VULKAN_BACKEND
			: vulkan::VulkanCommandList(queueType, renderInterface)
#endif
		{}

		auto barrier(
			const std::initializer_list<BarrierDescriptor>& descriptors) -> void
		{
			VALIDATE(validateBarrier(descriptors));
			barrierInternal(descriptors);
		}
		/*Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& descriptor);
		void endSplitBarrier(Handle<SplitBarrier> barrier);*/

		[[nodiscard]] auto buildAccelerationStructure(
			const TriangleGeometry& geometry,
			const std::vector<AccelerationStructureDescriptor>& descriptors) ->
		std::vector<Handle<AccelerationStructure>>
		{
			return buildAccelerationStructureInternal(geometry, descriptors);
		}

		[[nodiscard]] auto buildAccelerationStructure(
			const std::initializer_list<AccelerationStructureInstance>&
			instances) -> Handle<AccelerationStructure>
		{
			return buildAccelerationStructureInternal(instances);
		}

		//void beginRendering(const RenderingDescriptor& descriptor);
		auto beginRendering(
			const RenderingDescriptor& descriptor,
			const RenderArea& area) -> void
		{
			VALIDATE(validateBeginRendering(descriptor, area));
			beginRenderingInternal(descriptor, area);
		}
		auto endRendering() -> void
		{
			VALIDATE(validateEndRendering());
			endRenderingInternal();
		}

		auto begin() -> void
		{
			beginInternal();
		}

		auto end() -> void
		{
			endInternal();
		}

		auto bindGroup(core::u32 set, const Handle<BindGroup>& handle) -> void
		{
			bindGroupInternal(set, handle);
		}

		auto bindPipeline(const Handle<Pipeline>& pipeline) -> void
		{
			bindPipelineInternal(pipeline);
		}

		auto setScissor(const Scissor& scissor) -> void
		{
			VALIDATE(validateSetScissor(scissor));
			setScissorInternal(scissor);
		}

		auto setViewport(const Viewport& viewport) -> void
		{
			VALIDATE(validateSetViewport(viewport));
			setViewportInternal(viewport);
		}

		template <class ConstantValue>
		auto pushConstant(const ConstantValue& value)
		{
			pushConstantInternal(value);
		}

		auto bindIndexBuffer(const Handle<Buffer>& buffer, const core::u64 offset, const IndexType indexType) -> void
		{
			bindIndexBufferInternal(buffer, offset, indexType);
		}

		auto draw(
			core::u32 vertexCount,
			core::u32 instanceCount,
			core::u32 firstVertex,
			core::u32 firstInstance) -> void
		{
			//TODO: When scissor or viewport state was not set before, than make and set a Fullscreen scissor and viewport (or match render area)
			VALIDATE(validateDraw(vertexCount, instanceCount, firstVertex, firstInstance));
			drawInternal(vertexCount, instanceCount, firstVertex, firstInstance);
		}

		auto drawIndexed(
			core::u32 indexCount,
			core::u32 instanceCount,
			core::u32 firstIndex,
			core::i32 vertexOffset,
			core::u32 firstInstance
		) -> void
		{
			drawIndexedInternal(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		auto transfer(
			const SourceBufferDescriptor& srcBufferDescriptor,
			const DestinationImageDescriptor& dstImageDescription) -> void
		{
			transferInternal(srcBufferDescriptor, dstImageDescription);
		}

	protected:
		validation::CommandListValidator validatorObject_;

		friend RenderInterface;
	};
}
