cmake_minimum_required(VERSION 3.23)
project(ToyEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
if(MSVC)
    #add_compile_options("$<$<CONFIG:DEBUG>:/ZI>")
    #enables hot reload in visual studio
    add_link_options("$<$<CONFIG:DEBUG>:/INCREMENTAL>")
endif()

message(STATUS "toolchain: ${CMAKE_TOOLCHAIN_FILE}")

option(BUILD_TOY_ENGINE_MODULE_TESTS "Build module tests." on)
option(TOY_ENGINE_ENABLE_VULKAN_VALIDATION "Enable Vulkan validation layer." on)
option(TOY_ENGINE_ENABLE_GPU_PROFILING "Enable GPU profiling." off)
option(TOY_ENGINE_VULKAN_BACKEND "Enable Vulkan backend." on)
option(TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION "Enable renderer abstraction interface validation" on)
option(TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING "Enable RenderDoc captureing" off)
option(TOY_ENGINE_ENABLE_RAY_TRACING "Enable Hardware Ray Tracing support" off)

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
find_package(cityhash CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(g3log CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(MPMCQueue CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(crossguid CONFIG REQUIRED)

add_subdirectory(External EXCLUDE_FROM_ALL)

add_subdirectory(Engine EXCLUDE_FROM_ALL)
add_subdirectory(Application)
add_subdirectory(EditorApplication)
add_subdirectory(Tools)