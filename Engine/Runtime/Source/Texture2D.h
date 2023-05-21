#pragma once

#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>
namespace toy
{
	struct Texture2D
	{
		core::u32 width;
		core::u32 height;
		graphics::rhi::Handle<graphics::rhi::Image> image;
		graphics::rhi::Handle<graphics::rhi::ImageView> view;
		bool hasMips;
		core::u32 bytesPerTexel;
	};
}