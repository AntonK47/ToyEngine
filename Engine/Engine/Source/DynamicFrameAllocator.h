#pragma once

#include <RenderInterface.h>

namespace toy::graphics
{
	class LinearAllocator final
	{
		auto suballocate();
	};

	struct AllcatorDescriptor
	{
		toy::graphics::rhi::Buffer buffer;
	};


	class DynamicFrameAllcoator final
	{
		auto initialize(const AllocatorDescriptor& descriptor);

		[nodiscard] auto allocateAsLinearAllocator() -> LinearAllocator;

		auto nextFrame();
		auto flush();
	};
}

