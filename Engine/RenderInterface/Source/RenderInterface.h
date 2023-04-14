#pragma once
#include <vector>
#include <optional>
#include <concepts>
#include <ranges>
#include <concepts>

#include "RenderInterfaceTypes.h"

#include "RenderInterfaceValidator.h"
#include "CommandList.h"

#include "VulkanRHI/VulkanRenderInterface.h"
#include "QueueType.h"
#include "SubmitBatch.h"


namespace toy::graphics::rhi
{
	using namespace core;

	class RenderInterface final 
#ifdef TOY_ENGINE_VULKAN_BACKEND
		: public vulkan::VulkanRenderInterface
#endif

	{
	public:
		RenderInterface(const RenderInterface& other) = delete;
		RenderInterface(RenderInterface&& other) noexcept = default;

		RenderInterface& operator=(const RenderInterface& other) = default;
		RenderInterface& operator=(RenderInterface&& other) noexcept = default;
		
		RenderInterface() {}
		~RenderInterface() = default;

		[[nodiscard]] auto getNativeBackend() -> NativeBackend
		{
			return getNativeBackendInternal();
		}

		[[nodiscard]] auto requestMemoryBudget() -> MemoryBudget
		{
			return requestMemoryBudgetInternal();
		}
		
		auto initialize(const RendererDescriptor& descriptor) -> void
		{
			VALIDATE(validateInitialize(descriptor));
			initializeInternal(descriptor);
		}
		auto deinitialize() -> void
		{
			VALIDATE(validateDeinitialize());
			deinitializeInternal();
		}

		[[nodiscard]] auto acquireCommandList(
			QueueType queueType,
			const WorkerThreadId workerId = WorkerThreadId{ 0 },
			const UsageScope & usageScope = UsageScope::inFrame) -> CommandList
		{
			return acquireCommandListInternal(queueType, workerId, usageScope);
		}
		
		[[nodiscard]] auto submitCommandList(
			QueueType queueType,
			const std::initializer_list<CommandList> commandLists,
			const std::initializer_list<SubmitDependency> dependencies) -> SubmitBatch
		{
			return submitCommandListInternal(queueType, commandLists, dependencies);
		}

		[[nodiscard]] auto submitCommandList(
			QueueType queueType,
			const std::initializer_list<CommandList> commandLists,
			const std::span<SubmitDependency> dependencies) -> SubmitBatch
		{
			return submitCommandListInternal(queueType, commandLists, dependencies);
		}

		[[nodiscard]] auto submitCommandList(
			QueueType queueType,
			const std::span<CommandList> commandLists,
			const std::initializer_list<SubmitDependency> dependencies) -> SubmitBatch
		{
			return submitCommandListInternal(queueType, commandLists, dependencies);
		}

		[[nodiscard]] auto submitCommandList(
			QueueType queueType,
			const std::span<CommandList> commandLists,
			const std::span<SubmitDependency> dependencies) -> SubmitBatch
		{
			return submitCommandListInternal(queueType, commandLists, dependencies);
		}
		
		auto submitBatches(
			const QueueType queueType,
			const std::initializer_list<SubmitBatch> batches) -> void
		{
			submitBatchesInternal(queueType, batches);
		}

		auto submitBatches(
			const QueueType queueType,
			const std::span<SubmitBatch> batches) ->void
		{
			submitBatchesInternal(queueType, batches);
		}
		
		auto nextFrame() -> void
		{
			nextFrameInternal();
		}

		[[nodiscard]] auto acquireNextSwapchainImage() -> SwapchainImage
		{
			return acquireNextSwapchainImageInternal();
		}

		auto present(const SubmitDependency& dependency) -> void
		{
			presentInternal(dependency);
		}


		//TODO:: move create functions in resource manager
		//TODO: resource creation can be moved in a separate resource management class

		[[nodiscard]] auto createBuffer(
			const BufferDescriptor& descriptor, [[maybe_unused]] const DebugLabel label = {}) -> Handle<Buffer>
		{
			return createBufferInternal(descriptor, label);
		}

		auto destroyBuffer(const Handle<Buffer> handle) -> void
		{
			destroyBufferInternal(handle);
		}

		/*struct ResourceDescriptor{};
		std::initializer_list<std::initializer_list<Handle<>>> createAliasedResources(std::initializer_list<std::initializer_list<ResourceDescriptor>> aliasedResourceDescriptors);*/

		[[nodiscard]] auto createImage(const ImageDescriptor& descriptor) -> Handle<Image>
		{
			return createImageInternal(descriptor);
		}

		[[nodiscard]] auto createSampler(const SamplerDescriptor& descriptor, [[maybe_unused]] const DebugLabel label = {}) -> Handle<Sampler>
		{
			return createSamplerInternal(descriptor, label);
		}

		[[nodiscard]] auto createVirtualTexture(const VirtualTextureDescriptor& descriptor) -> Handle<VirtualTexture>
		{
			return createVirtualTextureInternal(descriptor);
		}

		[[nodiscard]] auto createImageView(const ImageViewDescriptor& descriptor) -> Handle<ImageView>
		{
			return createImageViewInternal(descriptor);
		}

		auto map(const Handle<Buffer>& buffer, void** data) -> void
		{
			mapInternal(buffer, data);
		}

		auto unmap(const Handle<Buffer>& buffer) -> void
		{
			unmapInternal(buffer);
		}

		[[nodiscard]] auto createBindGroupLayout(
			const BindGroupDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel& label = {}) -> Handle<BindGroupLayout>
		{
			return createBindGroupLayoutInternal(descriptor, label);
		}

		[[nodiscard]] auto allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			const UsageScope& scope = UsageScope::inFrame,
			[[maybe_unused]] const DebugLabel& label = {}) -> Handle<BindGroup>
		{
			//TODO: this should be thread safe
			return allocateBindGroupInternal(bindGroupLayout, 1, scope, label).front();
		}

		[[nodiscard]] auto allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const UsageScope& scope = UsageScope::inFrame,
			[[maybe_unused]] const DebugLabel& label = {}) ->
		std::vector<Handle<BindGroup>>//TODO: smallvector
		{
			return allocateBindGroupInternal(bindGroupLayout, bindGroupCount, scope, label);
		}

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		//virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;

		//draft for pipeline creation
		[[nodiscard]] auto createPipeline(
			const GraphicsPipelineDescriptor& graphicsPipelineDescriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {},
			const std::vector<PushConstant> pushConstants = {}) 
			-> Handle<Pipeline>
		{
			return createPipelineInternal(graphicsPipelineDescriptor, bindGroups, pushConstants);
		}

		[[nodiscard]] auto createPipeline(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {},
			const std::vector<PushConstant> pushConstants = {}) -> Handle<Pipeline>
		{
			return createPipelineInternal(descriptor, bindGroups, pushConstants);
		}

		//TODO: Why do not move bindGroups into a descriptor
		[[nodiscard]] auto createPipeline(
			const RayTracingPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {}) -> Handle<Pipeline>
		{
			return {};
		}

		[[nodiscard]] auto createShaderModule(
			ShaderStage stage,
			const ShaderCode& code) -> Handle<ShaderModule>
		{
			return createShaderModuleInternal(stage, code);
		}

		auto beginDebugLabel(const QueueType queueType, const DebugLabel& label) -> void
		{
			beginDebugLabelInternal(queueType, label);
		}

		auto endDebugLabel(const QueueType queueType) -> void
		{
			endDebugLabelInternal(queueType);
		}

		//TODO This function should be thread safe??????????, internally there are no storage write, but only read access
		auto updateBindGroup(
			const Handle<BindGroup>& bindGroup,
			const std::initializer_list<BindingDataMapping> mappings) -> void
		{
			updateBindGroupInternal(bindGroup, mappings);
		}

		auto updateBindGroup(
			const Handle<BindGroup>& bindGroup,
			const std::span<BindingDataMapping> mappings) -> void
		{
			updateBindGroupInternal(bindGroup, mappings);
		}

	private:
		DECLARE_VALIDATOR(validation::RenderInterfaceValidator);
	};
}
