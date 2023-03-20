#pragma once
#include <vector>
#include <glm/ext/matrix_common.hpp>
#include <glm/glm.hpp>

#include "CommandListValidator.h"
#include "RenderInterfaceCommonTypes.h"
#include "Resource.h"
#include "ValidationCommon.h"
#include <initializer_list>

namespace toy::renderer
{
	struct BindGroup;
	struct ImageResource;
	enum class QueueType;

	

	template <typename T>
	class Ref
	{
	public:
		explicit Ref(T* object) : object_{object}
		{}

		template <typename R>
		[[nodiscard]] const R& query() const
		{
			return *static_cast<const R*>(object_);
		}
	private:
		T* object_{};
	};

	enum class Layout
	{
		undefined,
		present,
		colorRenderTarget,
		depthStencilRenderTarget,
		transferSrc,
		transferDst,
		shaderRead,
		shaderReadWrite
	};

	enum class ShaderStageUsage
	{
		vertex,
		fragment,
		task
	};

	enum class ResourcePipelineStageUsageFlagBits : core::FlagBits
	{
		none = 0 << 0,
		vertex = 1 << 0,
		fragment = 1 << 1,
		compute = 1 << 2,
	};

	using ResourcePipelineStageUsageFlags = core::Flags<ResourcePipelineStageUsageFlagBits>;


	struct ImageBarrierDescriptor
	{
		Layout srcLayout;
		Layout dstLayout;
		ResourcePipelineStageUsageFlags srcStage{ ResourcePipelineStageUsageFlagBits::none }; //TODO: I don't like this design desition, because in contextes like data transfer it doesn't make sence.
		ResourcePipelineStageUsageFlags dstStage{ ResourcePipelineStageUsageFlagBits::none };
		ImageViewAspect aspect;
		Handle<Image> image;
		//???
	};
	struct MemoryBarrierDescriptor {};
	struct BufferBarrierDescriptor {};


	using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;

	struct SplitBarrier
	{
		/*
		 * in vulkan for instance, we have to store event and wait for it
		 */
	};

	enum class LoadOperation
	{
		load,
		clear,
		dontCare,
		none
	};

	enum class StoreOperation
	{
		store,
		dontCare,
		none
	};

	enum class ResolveMode
	{
		min,
		max,
		avg,
		none
	};

	struct ColorClear
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct DepthClear
	{
		float depth;
	};

	using ClearValue = std::variant<ColorClear, DepthClear>;

	struct RenderTargetDescriptor
	{
		Handle<ImageView> imageView{};
		LoadOperation load{};
		StoreOperation store{};
		ResolveMode resolveMode{};
		ClearValue clearValue{};
	};

	struct RenderingDescriptor
	{
		std::vector<RenderTargetDescriptor> colorRenderTargets{};//TODO: smallvector
		std::optional<RenderTargetDescriptor> depthRenderTarget{};
		std::optional<RenderTargetDescriptor> stencilRenderTarget{};
	};

	enum class GeometryBehavior
	{
		none,
		translucent,
		opaque,
		hitAnyOnce
	};

	struct TriangleGeometry
	{
		Handle<Buffer> indexBuffer;
		Handle<Buffer> vertexBuffer;
		core::u32 totalVertices{ 0 };
		core::u32 vertexStride{ 0 };
		GeometryBehavior behavior{ GeometryBehavior::none };
	};

	struct AccelerationStructureInstance
	{
		glm::mat3x4 transform{};
		core::u32 index{};
		core::u8 visibilityMask{};
		//...
		Handle<AccelerationStructure> blas;
	};

	struct SourceBufferDescrptor
	{
		Handle<Buffer> buffer;
		core::u32 offset;
	};

	struct Region
	{
		core::u32 mip{};
		core::u32 baseLayer{};
		core::u32 layerCount{};
		glm::uvec3 extent{};
		glm::ivec3 offset{ 0,0,0 };
	};

	struct DestinationImageDescriptor
	{
		Handle<Image> image;
		std::vector<Region> regions{};
	};
	
	enum class IndexType
	{
		index16,
		index32
	};

	/*template <typename T>
	concept CommandListConcept = requires(T cmd) {
		cmd.beginRenderingInternal(RenderingDescriptor{}, RenderArea{});
		cmd.endRenderingInternal();
		cmd.barrierInternal({ BarrierDescriptor{}, BarrierDescriptor{} });
		cmd.drawInternal(core::u32{}, core::u32{}, core::u32{}, core::u32{});
		cmd.bindPipelineInternal(Handle<Pipeline>{});
		cmd.setScissorInternal(Scissor{});
		cmd.setViewportInternal(Viewport{});
		cmd.bindGroupInternal(core::u32{}, Handle<BindGroup>{});
		{cmd.buildAccelerationStructureInternal(
			TriangleGeometry{},
			{ AccelerationStructureDescriptor{} })} -> std::convertible_to< std::vector<Handle<AccelerationStructure>>>;
		{cmd.buildAccelerationStructureInternal({ AccelerationStructureInstance{} })}->std::convertible_to<Handle<AccelerationStructure>>;

		cmd.beginInternal();
		cmd.endInternal();
	};

	template <CommandListConcept CommandListModel>
	class CommandList;

	

	template <typename T>
	concept RenderInterfaceConcept = CommandListConcept<typename T::CommandListType> &&
		requires(T renderer, CommandList<typename T::CommandListType> cmd)
	{
		renderer.initializeInternal(RendererDescriptor{});
		renderer.deinitializeInternal();
		{renderer.acquireCommandListInternal(QueueType{}, UsageScope{})} -> std::convertible_to<CommandList<typename T::CommandListType>>;
		renderer.submitCommandListInternal(cmd);
		{renderer.submitCommandListInternal(QueueType{},
			{ cmd },
			{ SubmitDependency{} })} -> std::convertible_to<SubmitDependency>;
		renderer.nextFrameInternal();
		{renderer.acquireNextSwapchainImageInternal()} -> std::convertible_to<SwapchainImage>;
		renderer.presentInternal();
		{renderer.getNativeBackendInternal()} -> std::convertible_to<NativeBackend>;
	};

	template <RenderInterfaceConcept RenderInterfaceModel>
	class RenderInterface;
	*/

	template <typename RenderInterfaceImplementation>
	class RenderInterface;

	template <typename CommandListImplementation, typename RenderInterfaceImplementation>
	class CommandList
	{
	private:
		CommandListImplementation& implementation()
		{
			return static_cast<CommandListImplementation&>(*this);
		}
	protected:
		CommandList(const QueueType queueType, RenderInterfaceImplementation& renderInterface) : queueType_(queueType), renderer_(renderInterface){}
	public:
		

		CommandList(const CommandList& other) = default;
		CommandList(CommandList&& other) noexcept = default;
		CommandList& operator=(const CommandList& other) = default;
		CommandList& operator=(CommandList&& other) noexcept = default;

		/*explicit CommandList(const QueueType queueType, CommandListHandle& nativeCommandListHandle);*/
		virtual ~CommandList() = default;

		auto barrier(
			const std::initializer_list<BarrierDescriptor>& descriptors) -> void
		{
			VALIDATE(validateBarrier(descriptors));
			implementation().barrierInternal(descriptors);
		}
		/*Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& descriptor);
		void endSplitBarrier(Handle<SplitBarrier> barrier);*/

		[[nodiscard]] auto buildAccelerationStructure(
			const TriangleGeometry& geometry,
			const std::vector<AccelerationStructureDescriptor>& descriptors) ->
		std::vector<Handle<AccelerationStructure>>
		{
			return implementation().buildAccelerationStructureInternal(geometry, descriptors);
		}

		[[nodiscard]] auto buildAccelerationStructure(
			const std::initializer_list<AccelerationStructureInstance>&
			instances) -> Handle<AccelerationStructure>
		{
			return implementation().buildAccelerationStructureInternal(instances);
		}

		//void beginRendering(const RenderingDescriptor& descriptor);
		auto beginRendering(const RenderingDescriptor& descriptor,
		                    const RenderArea& area) -> void
		{
			VALIDATE(validateBeginRendering(descriptor, area));
			implementation().beginRenderingInternal(descriptor, area);
		}
		auto endRendering() -> void
		{
			VALIDATE(validateEndRendering());
			implementation().endRenderingInternal();
		}

		auto begin() -> void
		{
			implementation().beginInternal();
		}

		auto end() -> void
		{
			implementation().endInternal();
		}

		auto bindGroup(core::u32 set, const Handle<BindGroup>& handle) -> void
		{
			implementation().bindGroupInternal(set, handle);
		}

		auto bindPipeline(const Handle<Pipeline>& pipeline) -> void
		{
			implementation().bindPipelineInternal(pipeline);
		}

		auto setScissor(const Scissor& scissor) -> void
		{
			VALIDATE(validateSetScissor(scissor));
			implementation().setScissorInternal(scissor);
		}

		auto setViewport(const Viewport& viewport) -> void
		{
			VALIDATE(validateSetViewport(viewport));
			implementation().setViewportInternal(viewport);
		}

		template <class ConstantValue>
		auto pushConstant(const ConstantValue& value)
		{
			implementation().pushConstantInternal(value);
		}

		auto bindIndexBuffer(const Handle<Buffer>& buffer, const core::u64 offset, const IndexType indexType) -> void
		{
			implementation().bindIndexBufferInternal(buffer, offset, indexType);
		}

		auto draw(
			core::u32 vertexCount,
			core::u32 instanceCount,
			core::u32 firstVertex,
			core::u32 firstInstance) -> void
		{
			//TODO: When scissor or viewport state was not set before, than make and set a fullscreen scissor and viewport (or match render area)
			VALIDATE(validateDraw(vertexCount, instanceCount, firstVertex, firstInstance));
			implementation().drawInternal(vertexCount, instanceCount, firstVertex, firstInstance);
		}

		auto drawIndexed(
			core::u32 indexCount,
			core::u32 instanceCount,
			core::u32 firstIndex,
			core::i32 vertexOffset,
			core::u32 firstInstance
		) -> void
		{
			implementation().drawIndexedInternal(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		auto trasfer(const SourceBufferDescrptor& srcBufferDescriptor, const DestinationImageDescriptor& dstImageDescription) -> void
		{
			implementation().transferInternal(srcBufferDescriptor, dstImageDescription);
		}

		[[nodiscard]] auto getQueueType() const -> QueueType { return queueType_; }

	protected:
		QueueType queueType_{};
		RenderInterfaceImplementation& renderer_;
		validation::CommandListValidator validatorObject_;
		//DECLARE_VALIDATOR(validation::CommandListValidator);TODO:!?
	};
}
