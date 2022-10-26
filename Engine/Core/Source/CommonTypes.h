#pragma once
#include <cstdint>


namespace toy::core
{
	using FlagBits = uint32_t;

	template<typename FlagBits>
	struct Flags
	{
		Flags(const FlagBits& bit)
		{
			*this |= (bit);
		}

		Flags& operator |=(const FlagBits& bit)
		{
			flags |= static_cast<uint32_t>(bit);
			return *this;
		}

		Flags& operator =(const FlagBits& bit)
		{
			flags = static_cast<uint32_t>(bit);
			return *this;
		}

		[[nodiscard]] bool containBit(const FlagBits& bit) const
		{
			return static_cast<bool>(flags & static_cast<uint32_t>(bit));
		}

	private:
		uint32_t flags{};
	};

	//TODO: create sfinae friendly operator
	/*template<typename T>
	Flags<T> operator|(const T& lhs, const T& rhs)
	{
		auto flag = FlagBits<T>{};
		flag |= lhs;
		flag |= rhs;
		return flag;
	}*/


	using u64 = uint64_t;
	using u32 = uint32_t;
	using u16 = uint16_t;
	using u8 = uint8_t;

	using i64 = int64_t;
	using i32 = int32_t;
	using i16 = int16_t;
	using i8 = int8_t;

	using f64 = double;
	using f32 = float;
}
