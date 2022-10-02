#include "RenderInterface.h"

#include <Hash.h>



using namespace toy::renderer;

namespace toy::renderer
{
	void RenderInterface::initialize(const RendererDescriptor& descriptor)
	{
		VALIDATE(validateInitialize(descriptor));
		initializeInternal(descriptor);
	}

	void RenderInterface::deinitialize()
	{
		VALIDATE(validateDeinitialize());
		deinitializeInternal();
	}

	std::unique_ptr<CommandList> RenderInterface::acquireCommandList(const QueueType queueType,
	                                                                 const CommandListType commandListType)
	{
		return acquireCommandListInternal(queueType, commandListType);
	}

	void RenderInterface::submitCommandList(std::unique_ptr<CommandList> commandList)
	{
		submitCommandListInternal(std::move(commandList));
	}

	void RenderInterface::nextFrame()
	{
		nextFrameInternal();
	}

	SwapchainImage RenderInterface::acquireNextSwapchainImage()
	{
		return acquireNextSwapchainImageInternal();
	}

	void RenderInterface::present()
	{
		presentInternal();
	}

	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout)
	{
		return {};
	}

	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor)
	{
		return {};
	}

	BindGroupLayout RenderInterface::allocateBindGroupLayout(const BindGroupDescriptor& descriptor)
	{

		const auto hash = Hasher::hash(descriptor);

		if(bindGroupLayoutCache_.contains(hash))
		{
			return bindGroupLayoutCache_[hash];
		}

		const auto groupLayout = allocateBindGroupLayoutInternal(descriptor);
		bindGroupLayoutCache_[hash] = groupLayout;

		return groupLayout;
	}

	CommandList::CommandList(const QueueType queueType): ownedQueueType_(queueType)
	{  }

	void CommandList::barrier(const std::initializer_list<BarrierDescriptor>& descriptors)
	{
		VALIDATE(validateBarrier(descriptors));
		barrierInternal(descriptors);
	}

	void CommandList::beginRendering(const RenderingDescriptor& descriptor, const RenderArea& area)
	{
		VALIDATE(validateBeginRendering(descriptor, area));
		beginRenderingInternal(descriptor, area);
	}

	void CommandList::endRendering()
	{
		VALIDATE(validateEndRendering());
		endRenderingInternal();
	}

	void CommandList::beginRendering(const RenderingDescriptor& descriptor)
	{
		//VALIDATE(beginRenderingValidation(descriptor));
		//beginRendering(descriptor, descriptor.)
	}
}
