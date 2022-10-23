#pragma once
#include <vector>

#include "CommandListValidator.h"

#include "Resource.h"
#include "RenderInterfaceCommonTypes.h"

namespace toy::renderer
{
	struct BindGroup;
	struct ImageResource;
	enum class QueueType;
	struct Pipeline {};
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
		ResourcePipelineStageUsageFlags srcStage{ ResourcePipelineStageUsageFlagBits::none };
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
		std::vector<RenderTargetDescriptor> colorRenderTargets{};
		RenderTargetDescriptor depthRenderTarget{};
		RenderTargetDescriptor stencilRenderTarget{};
	};

	

	class CommandList
	{
	public:
		CommandList(const CommandList& other) = delete;
		CommandList(CommandList&& other) noexcept = default;
		CommandList& operator=(const CommandList& other) = default;
		CommandList& operator=(CommandList&& other) noexcept = default;
		void bindGroup(core::u32 set, const Handle<BindGroup>& handle);

		explicit CommandList(QueueType queueType);
		virtual ~CommandList() = default;

		void barrier(const std::initializer_list<BarrierDescriptor>& descriptors);
		/*Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& descriptor);
		void endSplitBarrier(Handle<SplitBarrier> barrier);*/

		void beginRendering(const RenderingDescriptor& descriptor);
		void beginRendering(const RenderingDescriptor& descriptor, const RenderArea& area);
		void endRendering();

		void bindPipeline(const Handle<Pipeline>& pipeline);
		void setScissor(const Scissor& scissor);
		void setViewport(const Viewport& viewport);
		void draw(core::u32 vertexCount,
			core::u32  instanceCount,
			core::u32  firstVertex,
			core::u32  firstInstance);

	protected:

		virtual void bindGroupInternal(core::u32 set, const Handle<BindGroup>& handle) = 0;

		virtual void barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors) = 0;

		virtual void beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area) = 0;
		virtual void endRenderingInternal() = 0;

		virtual void drawInternal(core::u32 vertexCount,
			core::u32  instanceCount,
			core::u32  firstVertex,
			core::u32  firstInstance) = 0;

		virtual void bindPipelineInternal(const Handle<Pipeline>& pipeline) = 0;

		virtual void setScissorInternal(const Scissor& scissor) = 0;
		virtual void setViewportInternal(const Viewport& viewport) = 0;

		QueueType ownedQueueType_{};

	private:
		validation::CommandListValidator validatorObject_;
		//DECLARE_VALIDATOR(validation::CommandListValidator);
	};
}
