#pragma once

#include <initializer_list>
#include <variant>

#include "RenderInterfaceTypes.h"

namespace toy::graphics::rhi
{
	struct ImageBarrierDescriptor;
	struct BufferBarrierDescriptor;
	struct MemoryBarrierDescriptor;

	struct RenderingDescriptor;
	using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;
}

namespace toy::graphics::rhi::validation
{
	class CommandListValidator
	{
	public:
		bool validateBeginRendering(const RenderingDescriptor& descriptor, const RenderArea& area);
		bool validateEndRendering();
		bool validateBarrier(const std::initializer_list<BarrierDescriptor>& descriptors);
		bool validateSetScissor(const Scissor& scissor);
		bool validateSetViewport(const Viewport& viewport);
		bool validateDraw(core::u32 u32,
		                  core::u32 uint32,
		                  core::u32 firstVertex,
		                  core::u32 firstInstance);

	private:

		bool validateRenderingScope(const char* functionName);

		bool renderingHasStarted_{ false };
		RenderArea currentRenderArea_{};
	};
}
