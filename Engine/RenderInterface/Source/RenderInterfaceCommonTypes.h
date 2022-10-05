#pragma once
#include <Common.h>

namespace toy
{
	struct Rectangle2D
	{
		core::i32 x;
		core::i32 y;
		core::u32 width;
		core::u32 height;
	};
	using RenderArea = Rectangle2D;

	using Scissor = Rectangle2D;

	struct Viewport
	{
		float x;
		float y;
		float width;
		float height;
	};
}