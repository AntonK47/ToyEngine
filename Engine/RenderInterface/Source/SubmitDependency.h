#pragma once

#include "QueueType.h"

namespace toy::graphics::rhi
{
	using namespace core;

	struct SubmitDependency
	{
		QueueType queueDependency{ QueueType::graphics };
		core::u64 value{}; //R&D: this should be opaque, because of different graphics API's
	};
}