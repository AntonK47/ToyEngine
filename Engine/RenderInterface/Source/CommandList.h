#pragma once
#include <vector>
#include <memory>

#include "CommandListValidator.h"
#include "Common.h"
#include "Resource.h"

namespace toy::renderer
{
	struct ImageResource;
	enum class QueueType;

	enum class Layout
	{
		Undefined,
		Present,
		ColorRenderTarget,
		DepthStencilRenderTarget,
		TransferSrc,
		TransferDst,
		ShaderRead,
		ShaderReadWrite
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
		ImageResource* image;
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

	struct RenderTargetDescriptor
	{
		ImageView* renderTargetImageAccessor{};
		LoadOperation load{};
		StoreOperation store{};
		ResolveMode resolveMode{};
		ColorClear clearValue{};
	};

	struct RenderingDescriptor
	{
		std::vector<RenderTargetDescriptor> colorRenderTargets{};
		RenderTargetDescriptor depthRenderTarget{};
		RenderTargetDescriptor stencilRenderTarget{};
	};

	struct RenderArea
	{
		core::i32 x;
		core::i32 y;
		core::u32 width;
		core::u32 height;
	};

	class CommandList
	{
	public:
		CommandList(const CommandList& other) = delete;
		CommandList(CommandList&& other) noexcept = default;
		CommandList& operator=(const CommandList& other) = default;
		CommandList& operator=(CommandList&& other) noexcept = default;

		explicit CommandList(QueueType queueType);
		virtual ~CommandList() = default;

		void barrier(const std::initializer_list<BarrierDescriptor>& descriptors);
		/*Handle<SplitBarrier> beginSplitBarrier(const BarrierDescriptor& descriptor);
		void endSplitBarrier(Handle<SplitBarrier> barrier);*/

		void beginRendering(const RenderingDescriptor& descriptor);
		void beginRendering(const RenderingDescriptor& descriptor, const RenderArea& area);
		void endRendering();


		void draw(core::u32 vertexCount,
			core::u32  instanceCount,
			core::u32  firstVertex,
			core::u32  firstInstance);
	protected:

		virtual void barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors) = 0;

		virtual void beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area) = 0;
		virtual void endRenderingInternal() = 0;

		virtual void drawInternal(core::u32 vertexCount,
			core::u32  instanceCount,
			core::u32  firstVertex,
			core::u32  firstInstance) = 0;

		QueueType ownedQueueType_{};

	private:
		DECLARE_VALIDATOR(validation::CommandListValidator);
	};
}
