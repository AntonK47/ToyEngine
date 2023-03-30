#include "CommandListValidator.h"
#include "RenderInterface.h"
#include <Logger.h>

using namespace toy::graphics::rhi::validation;

#define RENDERING_SCOPE_CHECK_PASSED validateRenderingScope(__FUNCTION__)

bool CommandListValidator::validateBeginRendering(const RenderingDescriptor& descriptor,
	const RenderArea& area)
{
	if (renderingHasStarted_)
	{
		LOG(VALIDATION_FAILED) << "BeginRendering has been already called!";
		return false;
	}
	currentRenderArea_ = area;
	renderingHasStarted_ = true;
	return true;
}

bool CommandListValidator::validateEndRendering()
{
	if (!renderingHasStarted_)
	{
		LOG(VALIDATION_FAILED) << "BeginRendering should be called, before calling this function!";
		return false;
	}
	renderingHasStarted_ = false;
	return true;
}

bool CommandListValidator::validateBarrier(const std::initializer_list<BarrierDescriptor>& descriptors)
{
	return true;
}

bool CommandListValidator::validateSetScissor(const Scissor& scissor)
{
	if(scissor.x < 0 || scissor.y < 0)
	{
		LOG(VALIDATION_FAILED) << "x and y offsets should be positive integer numbers!";
		return false;
	}
	if (scissor.x > std::numeric_limits<const u32>::max() - scissor.width || scissor.y > std::numeric_limits<u32>::max() - scissor.height)
	{
		LOG(VALIDATION_FAILED) << "offset + extent cause integer overflow!";
		return false;
	}

	return RENDERING_SCOPE_CHECK_PASSED;
}

bool CommandListValidator::validateSetViewport(const Viewport& viewport)
{
	//TODO: check bounds, maybe require some device requested constant data
	return RENDERING_SCOPE_CHECK_PASSED;
}

bool CommandListValidator::validateDraw(core::u32 u32,
	core::u32 uint32,
	core::u32 firstVertex,
	core::u32 firstInstance)
{
	//TODO: scissor and viewport state should be already set
	return RENDERING_SCOPE_CHECK_PASSED;
}

bool CommandListValidator::validateRenderingScope(const char* functionName)
{
	if (!renderingHasStarted_)
	{
		LOG(VALIDATION_FAILED) << std::string{ functionName } << " should be called in rendering scope!";
		return false;
	}

	return true;
}
