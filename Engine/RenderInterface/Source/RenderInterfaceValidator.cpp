#include "RenderInterfaceValidator.h"
#include <Logger.h>

bool toy::renderer::validation::RenderInterfaceValidator::validateInitialize(const RendererDescriptor& descriptor)
{
	if(hasInitialized_)
	{
		LOG(VALIDATION_FAILED) << "This module is already initialized! Initialization function was called multiple times!";

		return false;
	}

	hasInitialized_ = true;
	return true;
}

bool toy::renderer::validation::RenderInterfaceValidator::validateDeinitialize()
{
	if(!hasInitialized_)
	{
		LOG(VALIDATION_FAILED) << "This module has to be initialized first!";
		return false;
	}
	hasInitialized_ = false;
	return true;
}
