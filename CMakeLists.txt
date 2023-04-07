cmake_minimum_required(VERSION 3.23)
#set(VCPKG_TARGET_TRIPLET x64-windows-static)

#set(VCPKG_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS_RELEASE} /MT")
#set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} /MTd")
#set(VCPKG_C_FLAGS_RELEASE "${VCPKG_C_FLAGS_RELEASE} /MT")
#set(VCPKG_C_FLAGS_DEBUG "${VCPKG_C_FLAGS_DEBUG} /MTd")

project(ToyEngine)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_BUILD_TYPE Release) #Debug, Release, RelWithDebInfo and MinSizeRel
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

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

#workaround for e2500 in zpp_bits
#add_compile_definitions(FMT_USE_NONTYPE_TEMPLATE_ARGS=0)

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
find_package(g3log CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(MPMCQueue CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Freetype REQUIRED)

include(FetchContent)
#imgui-node-editor  
FetchContent_Declare(imgui-node-editor
    GIT_REPOSITORY https://github.com/ocornut/imgui_node_editor.git
    GIT_TAG origin/fix-for-v189.4 #fix-for-v189.4
)
FetchContent_MakeAvailable(imgui-node-editor)

add_library(
    imgui-node-editor-static 
STATIC 
    ${imgui-node-editor_SOURCE_DIR}/crude_json.h
    ${imgui-node-editor_SOURCE_DIR}/crude_json.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.inl
    ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.inl
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_api.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.inl)
        
    target_link_libraries(
    imgui-node-editor-static
PRIVATE
    imgui::imgui)
install(TARGETS imgui-node-editor-static LIBRARY DESTINATION lib)


set_target_properties(imgui-node-editor-static PROPERTIES FOLDER Misc)



#list(APPEND CMAKE_MODULE_PATH "${imgui-node-editor_SOURCE_DIR}/misc/cmake-modules")
#find_package(imgui-node-editor CONFIG REQUIRED)
message(STATUS "imgui node editor: " ${imgui-node-editor_SOURCE_DIR})
add_subdirectory(Engine)
add_subdirectory(Application)
add_subdirectory(Tools)