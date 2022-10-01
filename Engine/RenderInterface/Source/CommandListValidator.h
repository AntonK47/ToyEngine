#pragma once
#include <assert.h>
#include <initializer_list>
#include <variant>

#define LOG(message)
#define TOY_ASSERT(expression) assert(expression)
#define TOY_ASSERT_BREAK(expression) if(expression) __debugbreak()

#ifdef RENDERER_VALIDATION
#define DECLARE_VALIDATOR(type) type validatorObject_{}
#define VALIDATE(expression) TOY_ASSERT_BREAK(validatorObject_.##expression)
#else
#define VALIDATE(expression)
#define DECLARE_VALIDATOR(type)
#endif

namespace toy
{
	namespace renderer
	{
		struct ImageBarrierDescriptor;
		struct BufferBarrierDescriptor;
		struct MemoryBarrierDescriptor;
		struct RenderArea;
		struct RenderingDescriptor;
		using BarrierDescriptor = std::variant<ImageBarrierDescriptor, BufferBarrierDescriptor, MemoryBarrierDescriptor>;
	}
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
		//TODO: Logger???
	};
}
