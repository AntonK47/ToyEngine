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

	//TODO: [#1] create sfinae friendly operator
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


	struct Color
	{
		Color(const u8 r, const u8 g, const u8 b):r_(r), g_(g), b_(b)
		{}

		[[nodiscard]] float r() const
		{
			return static_cast<float>(r_) / 255.0f;
		}

		[[nodiscard]] float g() const
		{
			return static_cast<float>(g_) / 255.0f;
		}

		[[nodiscard]] float b() const
		{
			return static_cast<float>(b_) / 255.0f;
		}
	private:
		u8 r_, g_, b_;
	};

	struct BasicColorsPalette
	{
		inline static Color red = { 200, 10, 10 };
	};

	struct DebugLabel
	{
		const char* name{};
		Color color{0,0,0};
	};
}
