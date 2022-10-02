#include "CommandListValidator.h"
#include "RenderInterface.h"
#include <Logger.h>

using namespace toy::renderer::validation;

bool CommandListValidator::validateBeginRendering(const RenderingDescriptor& descriptor,
	const RenderArea& area)
{
	if (renderingHasStarted_)
	{
		LOG(VALIDATION_FAILED) << "BeginRendering has been already called!";
		return false;
	}
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
