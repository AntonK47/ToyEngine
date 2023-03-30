#pragma once


#include "QueueType.h"

namespace toy::graphics::rhi
{
	class RenderInterface;
}

namespace toy::graphics::rhi::detail
{
	class CommandListBase
	{
	public:
		[[nodiscard]] auto getQueueType() const -> QueueType { return queueType_; }
	protected:
		CommandListBase(QueueType queueType, RenderInterface& renderInterface) : queueType_{ queueType }, rhi_{ renderInterface } {}

		QueueType queueType_{};
		RenderInterface& rhi_;
	};
}
