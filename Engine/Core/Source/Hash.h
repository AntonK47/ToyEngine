#pragma once

#include <city.h>
#include "CommonTypes.h"

namespace toy::core
{
	struct Hasher
	{
		template <typename T>
		static u64 hash64(const T& value)
		{
			return CityHash64(reinterpret_cast<const char*>(&value), sizeof(T));
		}

		template <typename T>
		static u32 hash32(const T& value)
		{
			return CityHash32(reinterpret_cast<const char*>(&value), sizeof(T));
		}
	};
}
