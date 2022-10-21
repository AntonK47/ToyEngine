#pragma once
#include <CommonTypes.h>

namespace toy::renderer
{
	

	template <typename T>
	struct Handle
	{
		core::u32 index{};
	};
	

	enum class ImageViewType
	{
		_1D,
		_2D,
		_3D
	};

	enum class ImageViewAspect
	{
		color,
		depth
	};

	struct ImageView{};

	struct Image{};

	struct Buffer{};

	template <typename T>
	struct Accessor
	{
		/*T* resource;
		View<T> view;*/
	};
}