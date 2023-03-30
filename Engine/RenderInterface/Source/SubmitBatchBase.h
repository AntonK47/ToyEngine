#pragma once

#include "QueueType.h"

namespace toy::graphics::rhi
{
	class SubmitBatchBase
	{
	protected:
		explicit SubmitBatchBase(const QueueType type)
			: queueType_(type)
		{}
		QueueType queueType_;
	};
}
