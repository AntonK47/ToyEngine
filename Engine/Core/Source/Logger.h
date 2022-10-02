#pragma once
#include <g3log/g3log.hpp>

inline extern const LEVELS VULKAN_VALIDATION_ERROR{ WARNING.value + 1, {"VULKAN_VALIDATION_ERROR_LEVEL"} };
inline extern const LEVELS VALIDATION_FAILED{ WARNING.value + 2, {"VALIDATION_FAILED_LEVEL"} };
namespace toy::core::logger
{
    void initialize();
    void deinitialize();
}
