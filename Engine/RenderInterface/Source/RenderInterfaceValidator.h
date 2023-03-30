#pragma once
#include "ValidationCommon.h"

namespace toy::graphics::rhi
{
	struct RendererDescriptor;
}

namespace toy::graphics::rhi::validation
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