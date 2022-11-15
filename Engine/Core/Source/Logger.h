#pragma once
#include <g3log/g3log.hpp>

#undef LOG
struct NoopStream
{};

template<typename T>
NoopStream& operator<<(NoopStream& noop, const T& obj)
{
    return noop;
}
static NoopStream noopStream;
#define LOG(...) noopStream


inline extern const LEVELS VULKAN_VALIDATION_ERROR{ WARNING.value + 1, {"VULKAN_VALIDATION_ERROR_LEVEL"} };
inline extern const LEVELS VALIDATION_FAILED{ WARNING.value + 2, {"VALIDATION_FAILED_LEVEL"} };
namespace toy::core::logger
{
    void initialize();
    void deinitialize();
}
