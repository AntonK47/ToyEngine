#pragma once

#include <RenderInterface.h>

namespace toy::graphics
{
	class LinearAllocator final
	{
		auto suballocate();
	};

	struct AllocatorDescriptor
	{
		toy::graphics::rhi::Buffer buffer;
	};


	class DynamicFrameAllocator final
	{
		auto initialize(const AllocatorDescriptor& descriptor);

		[nodiscard] auto allocateAsLinearAllocator() -> LinearAllocator;

		auto nextFrame();
		auto flush();
	};
}

