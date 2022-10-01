#pragma once

#include <city.h>
#include "CommonTypes.h"

namespace toy::core
{
	struct Hasher
	{
		template <typename T>
		static u64 hash(const T& value)
		{
			return CityHash64(reinterpret_cast<const char*>(&value), sizeof(T));
		}
	};
}
