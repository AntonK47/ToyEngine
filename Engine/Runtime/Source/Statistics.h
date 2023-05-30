#pragma once

#include <Core.h>

namespace toy
{
	struct SceneDrawStatistics
	{
		core::u32 drawCalls{};
		core::u32 totalTrianglesCount{};
	};

	struct GuiDrawStatistics
	{
		core::u32 drawCalls{};
		core::u32 totalIndicesCount{};
		core::u32 totalVerticesCount{};
	};

	struct DrawStatistics
	{
		SceneDrawStatistics scene{};
		GuiDrawStatistics gui{};
	};
}