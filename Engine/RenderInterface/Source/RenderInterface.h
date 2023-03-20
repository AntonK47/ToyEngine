#pragma once
#include <vector>
#include <optional>

#include "BindGroupAllocator.h"
#include "RenderInterfaceCommonTypes.h"

#include "Resource.h"
#include "RenderInterfaceValidator.h"
#include "CommandList.h"

namespace toy::renderer
{
	using namespace core;

	struct SubmitDependency
	{
		QueueType queueDependency{ QueueType::graphics };
		core::u64 value{}; //R&D: this should be opaque, because of different graphics API's
	};

	template <typename SubmitBatchImplementation>
	class SubmitBatch
	{
	public:
		[[nodiscard]] auto barrier() -> SubmitDependency
		{
			return implementation().barrierInternal();
		}

	protected:
		explicit SubmitBatch(const QueueType type)
			: queueType_(type)
		{}

		auto implementation() -> SubmitBatchImplementation&
		{
			return static_cast<SubmitBatchImplementation&>(*this);
		}
		QueueType queueType_;
	};

	template<typename T>
	struct CommandListType
	{
		using type = typename T;
	};

	template<typename T>
	struct SubmitBatchType
	{
		using type = typename T;
	};

	struct WorkerThreadId
	{
		core::u32 index;
	};

	template <typename RenderInterfaceImplementation>
	class RenderInterface
	{

		using CommandListType = typename CommandListType<RenderInterfaceImplementation>::type;
		using SubmitBatchType = typename SubmitBatchType<RenderInterfaceImplementation>::type;
	private:
		auto implementation() -> RenderInterfaceImplementation&
		{
			return static_cast<RenderInterfaceImplementation&>(*this);
		}
	public:
		RenderInterface(const RenderInterface& other) = delete;
		RenderInterface(RenderInterface&& other) noexcept = default;

		RenderInterface& operator=(const RenderInterface& other) = default;
		RenderInterface& operator=(RenderInterface&& other) noexcept = default;
		
		RenderInterface(){}
		~RenderInterface() = default;

		[[nodiscard]] auto getNativeBackend() -> NativeBackend
		{
			return implementation().getNativeBackendInternal();
		}

		auto initialize(const RendererDescriptor& descriptor)
		{
			VALIDATE(validateInitialize(descriptor));
			implementation().initializeInternal(descriptor);
		}
		auto deinitialize()
		{
			VALIDATE(validateDeinitialize());
			implementation().deinitializeInternal();
		}

		[[nodiscard]] auto acquireCommandList(
			QueueType queueType, const WorkerThreadId workerId = WorkerThreadId{ 0 }, const UsageScope & usageScope = UsageScope::inFrame) -> CommandListType
		{
			return implementation().acquireCommandListInternal(queueType, workerId, usageScope);
		}
		

		auto submitCommandList(const CommandListType& commandList)
		{
			implementation().submitCommandListInternal(commandList);
		}
		
		[[nodiscard]] auto submitCommandList(
			QueueType queueType,
			const std::initializer_list<CommandListType>& commandLists,
			const std::initializer_list<SubmitDependency>& dependencies) -> SubmitBatchType
		{
			return implementation().submitCommandListInternal(queueType, commandLists, dependencies);
		}

		void submitBatches(const QueueType queueType, const std::initializer_list<SubmitBatchType>& batches)
		{
			implementation().submitBatchesInternal(queueType, batches);
		}
		
		void nextFrame()
		{
			implementation().nextFrameInternal();
		}

		[[nodiscard]] auto acquireNextSwapchainImage() -> SwapchainImage
		{
			return implementation().acquireNextSwapchainImageInternal();
		}

		void present(const SubmitDependency& dependency)
		{
			implementation().presentInternal(dependency);
		}


		//TODO:: move create functions in resource manager
		//TODO: resource creation can be moved in a separate resource management class

		[[nodiscard]] Buffer createBuffer(
			const BufferDescriptor& descriptor, [[maybe_unused]] const DebugLabel label = {})
		{
			return Buffer
			{
				.nativeHandle = implementation().createBufferInternal(descriptor, label),
				.size = descriptor.size,
		#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
				.debugLabel = label
		#endif
			};
		}

		/*struct ResourceDescriptor{};
		std::initializer_list<std::initializer_list<Handle<>>> createAliasedResources(std::initializer_list<std::initializer_list<ResourceDescriptor>> aliasedResourceDescriptors);*/

		[[nodiscard]] auto createImage(
			const ImageDescriptor& descriptor) -> Handle<Image>
		{
			return implementation().createImageInternal(descriptor);
		}

		[[nodiscard]] auto createSampler(const SamplerDescriptor& descriptor, [[maybe_unused]] const DebugLabel label = {}) -> Handle<Sampler>
		{
			return implementation().createSamplerInternal(descriptor, label);
		}

		[[nodiscard]] auto createVirtualTexture(const VirtualTextureDescriptor& descriptor) -> Handle<VirtualTexture>
		{
			return implementation().createVirtualTextureInternal(descriptor);
		}

		[[nodiscard]] auto createImageView(
			const ImageViewDescriptor& descriptor) -> Handle<ImageView>
		{
			return implementation().createImageViewInternal(descriptor);
		}

		auto map(const Handle<Buffer>& buffer, void** data) -> void
		{
			implementation().mapInternal(buffer, data);
		}

		auto unmap(const Handle<Buffer>& buffer) -> void
		{
		}

		[[nodiscard]] auto createBindGroupLayout(
			const BindGroupDescriptor& descriptor) -> Handle<BindGroupLayout>
		{
			return implementation().createBindGroupLayoutInternal(descriptor);
		}

		[[nodiscard]] auto allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			const UsageScope& scope = UsageScope::inFrame) -> Handle<BindGroup>
		{
			//TODO: this should be thread safe
			return implementation().allocateBindGroupInternal(bindGroupLayout, 1, scope).front();
		}

		[[nodiscard]] auto allocateBindGroup(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const UsageScope& scope = UsageScope::inFrame) ->
		std::vector<Handle<BindGroup>>//TODO: smallvector
		{
			return implementation().allocateBindGroupInternal(bindGroupLayout, bindGroupCount, scope);
		}

		//this function should be thread save????
		//Do I need make multi threaded resource creation? It can depend on Frame Graph resource management.
		//virtual Handle<RenderTarget> createRenderTarget(RenderTargetDescriptor) = 0;

		//draft for pipeline creation
		[[nodiscard]] auto createPipeline(
			const GraphicsPipelineDescriptor& graphicsPipelineDescriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {}, const std::vector<PushConstant> pushConstants = {}) -> Handle<
			Pipeline>
		{
			return implementation().createPipelineInternal(graphicsPipelineDescriptor, bindGroups, pushConstants);
		}

		[[nodiscard]] auto createPipeline(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {}) -> Handle<
			Pipeline>
		{
			return implementation().createPipelineInternal(descriptor, bindGroups);
		}

		//TODO: Why do not move bindGroups into a descriptor

		[[nodiscard]] auto createPipeline(
			const RayTracingPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups = {}) -> Handle<
			Pipeline>
		{
			return {};
		}

		[[nodiscard]] auto createShaderModule(
			ShaderStage stage,
			const ShaderCode& code) -> Handle<ShaderModule>
		{
			return implementation().createShaderModuleInternal(stage, code);
		}

		auto beginDebugLable(const QueueType queueType, const DebugLabel& label) -> void
		{
			implementation().beginDebugLableInternal(queueType, label);
		}

		auto endDebugLable(const QueueType queueType) -> void
		{
			implementation().endDebugLableInternal(queueType);
		}


		//TODO This function should be thread safe??????????, internaly there are no storage write, but only read access
		auto updateBindGroup(
			const Handle<BindGroup>& bindGroup,
			const std::initializer_list<BindingDataMapping>& mappings) -> void
		{
			implementation().updateBindGroupInternal(bindGroup, mappings);
		}

	private:
		DECLARE_VALIDATOR(validation::RenderInterfaceValidator);
	};
}
