#pragma once
#include "CommonTypes.h"

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

struct NativeBackend
{
	//depends on backend it should be converted to appropriate pointer type
	void* nativeBackend;
};