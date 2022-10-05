#include "CommandList.h"

#include "ValidationCommon.h"

using namespace toy::core;

namespace toy::renderer
{
		
	CommandList::CommandList(const QueueType queueType) : ownedQueueType_(queueType)
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

	void CommandList::bindPipeline(const Ref<Pipeline>& pipeline)
	{
		bindPipelineInternal(pipeline);
	}

	void CommandList::setScissor(const Scissor& scissor)
	{
		VALIDATE(validateSetScissor(scissor));
		setScissorInternal(scissor);
	}

	void CommandList::setViewport(const Viewport& viewport)
	{
		VALIDATE(validateSetViewport(viewport));
		setViewportInternal(viewport);
	}

	void CommandList::draw(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
	{
		//TODO: When scissor or viewport state was not set before, than make and set a fullscreen scissor and viewport (or match render area)
		VALIDATE(validateDraw(vertexCount, instanceCount, firstVertex, firstInstance));
		drawInternal(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::beginRendering(const RenderingDescriptor& descriptor)
	{
		//VALIDATE(beginRenderingValidation(descriptor));
		//beginRendering(descriptor, descriptor.)
	}

}