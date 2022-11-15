cmake_minimum_required(VERSION 3.23)
project(ToyEngine)


option(BUILD_TOY_ENGINE_MODULE_TESTS "Build module tests." on)
option(TOY_ENGINE_ENABLE_VULKAN_VALIDATION "Enable Vulkan validation layer." on)
option(TOY_ENGINE_ENABLE_GPU_PROFILING "Enable GPU profiling." off)
option(TOY_ENGINE_VULKAN_BACKEND "Enable Vulkan backend." on)
option(TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION "Enable renderer abstraction interface validation" on)
option(TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING "Enable RenderDoc captureing" on)
option(TOY_ENGINE_ENABLE_RAY_TRACING "Enable Hardware Ray Tracing support" on)

set(TOY_ADDITIONAL_DEFINES
    TOY_ENGINE_ENABLE_GPU_PROFILING
    TOY_ENGINE_ENABLE_VULKAN_VALIDATION
    TOY_ENGINE_VULKAN_BACKEND
    TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
    TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING
    TOY_ENGINE_ENABLE_RAY_TRACING
) 


define_property(GLOBAL PROPERTY TOY_SOURCE_DIR)

foreach(TOY_DEFINE_ENTRY ${TOY_ADDITIONAL_DEFINES})
    if(${${TOY_DEFINE_ENTRY}})
        message(STATUS "Additional compiler defines: " ${TOY_DEFINE_ENTRY})
        add_compile_definitions(${TOY_DEFINE_ENTRY})
    endif()
endforeach()

 find_package(Vulkan REQUIRED)
 find_package(GTest CONFIG REQUIRED)
 find_package(cityhash CONFIG REQUIRED)
 
 include(FetchContent)
FetchContent_Declare(
  g3log
  GIT_REPOSITORY https://github.com/KjellKod/g3log.git
)
set(G3_SHARED_RUNTIME OFF)
set(G3_SHARED_LIB OFF)
FetchContent_MakeAvailable(g3log)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(Engine)
add_subdirectory(Application)
add_subdirectory(Tools)