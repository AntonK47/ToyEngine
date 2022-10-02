#pragma once
#include "ValidationCommon.h"

namespace toy::renderer
{
	struct RendererDescriptor;
}

namespace toy::renderer::validation
{
	class RenderInterfaceValidator
	{
	public:
		bool validateInitialize(const RendererDescriptor& descriptor);
		bool validateDeinitialize();
	private:
		bool hasInitialized_{false};
	};
}