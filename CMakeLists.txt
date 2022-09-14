cmake_minimum_required(VERSION 3.23)
project(ToyEngine)


OPTION(BUILD_TOY_ENGINE_MODULE_TESTS "Build module tests." ON)
OPTION(TOY_ENGINE_ENABLE_VULKAN_VALIDATION "Enable Vulkan validation layer." ON)
OPTION(TOY_ENGINE_ENABLE_GPU_PROFILING "Enable GPU profiling." OFF)
OPTION(TOY_ENGINE_VULKAN_BACKEND "Enable Vulkan backend." ON)

set(TOY_ADDITIONAL_DEFINES
    TOY_ENGINE_ENABLE_GPU_PROFILING
    TOY_ENGINE_ENABLE_VULKAN_VALIDATION
    TOY_ENGINE_VULKAN_BACKEND
) 

foreach(TOY_DEFINE_ENTRY ${TOY_ADDITIONAL_DEFINES})
    if(${${TOY_DEFINE_ENTRY}})
        message(STATUS "Additional compiler defines: " ${TOY_DEFINE_ENTRY})
        add_compile_definitions(${TOY_DEFINE_ENTRY})
    endif()
endforeach()

 find_package(Vulkan REQUIRED)
 find_package(GTest CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(Engine)
add_subdirectory(Application)