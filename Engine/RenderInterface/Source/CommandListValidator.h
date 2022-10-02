#pragma once

#include <initializer_list>
#include <variant>

#include "ValidationCommon.h"


namespace toy::renderer
{
	struct ImageBarrierDescriptor;
	struct BufferBarrierDescriptor;
	struct MemoryBarrierDescriptor;
	struct RenderArea;
	struct RenderingDescriptor;
	using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;
}

namespace toy::renderer::validation
{
	class CommandListValidator
	{
	public:
		bool validateBeginRendering(const RenderingDescriptor& descriptor, const RenderArea& area);
		bool validateEndRendering();
		bool validateBarrier(const std::initializer_list<BarrierDescriptor>& descriptors);

	private:
		bool renderingHasStarted_{ false };
	};
}
