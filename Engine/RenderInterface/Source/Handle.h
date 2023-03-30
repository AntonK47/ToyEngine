#pragma once

#include <Core.h>

namespace toy::graphics::rhi
{
	template <typename T>
	struct Handle
	{
		core::u32 index{};
	};
}