#pragma once

namespace toy::renderer
{
	template <typename T>
	struct Handle
	{
		core::u32 index{};
	};

	struct Resource
	{};

	struct ImageResource : Resource
	{

	};

	struct ImageView : Resource
	{

	};

	template <typename T>
	struct Accessor
	{
		/*T* resource;
		View<T> view;*/
	};
}