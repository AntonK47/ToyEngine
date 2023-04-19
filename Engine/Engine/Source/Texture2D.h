#pragma once

#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>
namespace toy
{
	struct Texture2D
	{
		u32 width;
		u32 height;
		Handle<Image> image;
		Handle<ImageView> view;
		bool hasMips;
		u32 bytesPerTexel;
	};
}