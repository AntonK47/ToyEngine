#pragma once

namespace toy::renderer
{
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