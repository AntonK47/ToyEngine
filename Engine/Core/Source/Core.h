#pragma once

#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <assert.h>

struct NativeBackend
{
	//depends on backend it should be converted to appropriate pointer type
	void* nativeBackend;
};

template <typename R, typename V>
concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<std::remove_cvref_t<R>>, V>;

namespace toy::window
{
	struct WindowHandler
	{
#ifdef WIN32
		HWND hwnd;
		HINSTANCE hinstance;
#endif
	};

	struct BackendRendererMeta
	{
#ifdef TOY_ENGINE_VULKAN_BACKEND
		std::vector<const char*> requiredExtensions;
#endif
	};
}

namespace toy::core
{
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

	using FlagBits = u32;

	template<typename FlagBits>
	struct Flags
	{
		Flags(const FlagBits& bit)
		{
			*this |= (bit);
		}

		Flags& operator |=(const FlagBits& bit)
		{
			flags |= static_cast<u32>(bit);
			return *this;
		}

		Flags& operator =(const FlagBits& bit)
		{
			flags = static_cast<u32>(bit);
			return *this;
		}

		[[nodiscard]] bool containBit(const FlagBits& bit) const
		{
			return static_cast<bool>(flags & static_cast<u32>(bit));
		}

	private:
		u32 flags{};
	};




	struct Color
	{
		Color(const u8 r, const u8 g, const u8 b) :r_(r), g_(g), b_(b)
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
		Color color{ 0,0,0 };
	};

	using UID = u32;

	struct UIDGenerator
	{
		inline static auto generate() -> const UID
		{
			return id++;
		}
	private:
		inline static UID id{1};
	};

#define TOY_ASSERT(expression) assert(expression)
#define TOY_ASSERT_BREAK(expression) if(!(expression)) __debugbreak()
}

