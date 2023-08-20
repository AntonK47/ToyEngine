#cmake_minimum_required(VERSION 3.10)
#
#
#
#set(OPTION_CMP_GTC OFF)
#set(OPTION_BUILD_BROTLIG OFF)
#set(OPTION_CMP_APC OFF)
#set(CMP_HOST_WINDOWS ON)
#
#
#FetchContent_Declare(compressonator
#    GIT_REPOSITORY https://github.com/GPUOpen-Tools/compressonator.git
#    SOURCE_SUBDIR cmp_framework
#)
#
#if(NOT compressonator_POPULATED)
#    FetchContent_Populate(compressonator)
#  #add_subdirectory(${compressonator_SOURCE_DIR} ${catch_BINARY_DIR})
#
#add_library(CMP_Framework STATIC "")
#target_compile_features(CMP_Framework PUBLIC cxx_std_17)
#
#if(CMP_HOST_WINDOWS)
#    target_compile_definitions(CMP_Framework PUBLIC
#        -DCMP_USE_XMMINTRIN
#        )
#endif()
#
#file(GLOB_RECURSE HALF_SRC
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/half/*.h
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/half/*.cpp
#        )
#file(GLOB_RECURSE DDS_SRC
#        ${compressonator_SOURCE_DIR}/applications/_plugins/cimage/dds/*.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/cimage/dds/*.cpp
#        )
#file(GLOB_RECURSE HPC_SRC
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_encode/hpc/*.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_encode/hpc/*.cpp
#        )
#file(GLOB_RECURSE CORE_SHADERS_SRC
#        ${compressonator_SOURCE_DIR}/cmp_core/shaders/*.h
#        ${compressonator_SOURCE_DIR}/cmp_core/shaders/*.cpp
#        )
#file(GLOB_RECURSE CORE_SRC
#        ${compressonator_SOURCE_DIR}/cmp_core/source/*.h
#        ${compressonator_SOURCE_DIR}/cmp_core/source/*.cpp
#        )
#
#file(GLOB_RECURSE CMP_FRAMEWORK_SRC
#        ${compressonator_SOURCE_DIR}/cmp_framework/*.h
#        ${compressonator_SOURCE_DIR}/cmp_framework/*.cpp
#        )
#file(GLOB_RECURSE CMP_FRAMEWORK_COMMON_SRC
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/*.h
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/*.cpp
#        )
#
#target_sources(CMP_Framework
#        PRIVATE
#        ${HALF_SRC}
#        ${DDS_SRC}
#        ${HPC_SRC}
#        ${CORE_SHADERS_SRC}
#        ${CORE_SRC}
#        ${CMP_FRAMEWORK_SRC}
#        ${CMP_FRAMEWORK_COMMON_SRC}
#        
#        # CMP_Math
#        ${compressonator_SOURCE_DIR}/applications/_libs/cmp_math/cmp_math_common.h
#        ${compressonator_SOURCE_DIR}/applications/_libs/cmp_math/cmp_math_common.cpp
#        ${compressonator_SOURCE_DIR}/applications/_libs/cmp_math/cmp_math_cpuid.h
#        ${compressonator_SOURCE_DIR}/applications/_libs/cmp_math/cmp_math_cpuid.cpp
#        
#        # CMP_COMMON
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/atiformats.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/atiformats.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/pluginbase.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/plugininterface.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/pluginmanager.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/pluginmanager.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/query_timer.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/query_timer.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/stb_image.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/stb_image_write.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/tc_pluginapi.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/tc_plugininternal.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/tc_plugininternal.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/utilfuncs.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/utilfuncs.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/cmp_fileio.h 
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/cmp_fileio.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/format_conversion.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common/format_conversion.cpp
#
#        # CMP_SDK Lossy Compression
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bcn.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bcn.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc1/bc1.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc1/bc1.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc2/bc2.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc2/bc2.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc3/bc3.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc3/bc3.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc4/bc4.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc4/bc4.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc5/bc5.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc5/bc5.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc6/bc6h.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc6/bc6h.cpp
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc7/bc7.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/bc7/bc7.cpp
#        )
#
#
#if (OPTION_CMP_GTC)
#target_sources(CMP_Framework
#        PRIVATE
#        # Extra 
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/gtc/gtc.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/gtc/gtc.cpp
#        )
#endif()
#
#if (OPTION_BUILD_BROTLIG)
#    target_sources(CMP_Framework
#        PRIVATE
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/brlg/codec_brlg.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/brlg/codec_brlg.cpp
#    )
#
#    target_link_libraries(CMP_Framework
#        PRIVATE
#        ExtBrotlig
#    )
#endif()
#
#if (OPTION_CMP_APC)
#target_sources(CMP_Framework
#        PRIVATE
#        # Extra 
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/apc/apc.h
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk/apc/apc.cpp
#        )
#endif()
#
#target_include_directories(CMP_Framework
#        PRIVATE
#        ./
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/half
#        ${compressonator_SOURCE_DIR}/cmp_framework/common/
#        ${compressonator_SOURCE_DIR}/cmp_framework
#        ${compressonator_SOURCE_DIR}/cmp_core/source
#        ${compressonator_SOURCE_DIR}/cmp_core/shaders
#        ${compressonator_SOURCE_DIR}/cmp_compressonatorlib
#        ${compressonator_SOURCE_DIR}/cmp_compressonatorlib/buffer
#        ${compressonator_SOURCE_DIR}/applications/_plugins/ccmp_sdk
#        ${compressonator_SOURCE_DIR}/applications/_plugins/common
#        #${compressonator_SOURCE_DIR}/applications/_plugins/common/gltf
#        ${compressonator_SOURCE_DIR}/applications/_libs/cmp_math
#        ${compressonator_SOURCE_DIR}/applications/_libs/gpu_decode
#        )
#
#if (UNIX)
#    target_compile_definitions(CMP_Framework PRIVATE _LINUX)
#endif()
#
#set_target_properties(CMP_Framework PROPERTIES FOLDER "Libs")
#
#message(STATUS "Compressonator done")
#
#endif()



#cmake_minimum_required(VERSION 3.10)
#
#FetchContent_Declare(compressonator
#    GIT_REPOSITORY https://github.com/GPUOpen-Tools/compressonator.git
#    SOURCE_SUBDIR cmp_framework
#)
#
#if(NOT compressonator_POPULATED)
#    FetchContent_Populate(compressonator)
#  #add_subdirectory(${compressonator_SOURCE_DIR} ${catch_BINARY_DIR})
#endif()
#if(POLICY CMP0091)
#   cmake_policy(SET CMP0091 NEW)
#endif()
#
#if(POLICY CMP0076)
#  cmake_policy(SET CMP0076 NEW)
#endif()
#
#set(CMP_VERSION_MAJOR 4)
#set(CMP_VERSION_MINOR 3)
#
#if (NOT CMP_VERSION_BUILD_NUMBER)
#    set(CMP_VERSION_BUILD_NUMBER 0)
#endif()
#
#
#option(OPTION_BUILD_APPS_CMP_CLI       OFF)
#option(OPTION_BUILD_APPS_CMP_GUI       OFF)
#option(OPTION_BUILD_APPS_CMP_TESTCORE  OFF)
#option(OPTION_BUILD_APPS_CMP_EXAMPLES  OFF)
#
#
#
# set(LIB_BUILD_COMPRESSONATOR_SDK ON)
# set(LIB_BUILD_FRAMEWORK_SDK ON)
#
#
#
#
#
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Supported configuration options" FORCE)
#
#
## '_DEBUG' is only defined by Visual Studio. Make sure it's definied everywhere
#add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
#
#
#
#set(CMAKE_CXX_STANDARD 17)
#add_compile_definitions(_CMP_CPP17_=1)
#
#
#add_compile_definitions(_WIN32)
#
#set(CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO")
#
#
#if (LIB_BUILD_CORE)
#    add_subdirectory(${compressonator_SOURCE_DIR}/cmp_core cmp)
#endif()
#
#if (LIB_BUILD_FRAMEWORK_SDK)
#    add_subdirectory(${compressonator_SOURCE_DIR}/cmp_framework cmp_f)
#endif()
#
#if (LIB_BUILD_COMPRESSONATOR_SDK)
#    add_subdirectory(${compressonator_SOURCE_DIR}/cmp_compressonatorlib cmp_lib)
#endif()
#

#FetchContent_Declare(compressonator
#    OPTION_BUILD_BROTLIG OFF
#    GIT_REPOSITORY https://github.com/GPUOpen-Tools/compressonator.git
#    SOURCE_SUBDIR cmp_framework
#)
#
#set(OPTION_ENABLE_ALL_APPS OFF)
#set(OPTION_BUILD_APPS_CMP_VISION ON)
#set(OPTION_BUILD_BROTLIG OFF)
#FetchContent_MakeAvailable(compressonator)
#message(STATUS "................ ${compressonator_SOURCE_DIR}")

#execute_process(COMMAND python ${compressonator_SOURCE_DIR}/scripts/fetch_dependencies.py
#    ${compressonator_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})
#
##
#cmake_minimum_required(VERSION 3.10)

#set_target_properties(compressonator-static PROPERTIES
#  INTERFACE_INCLUDE_DIRECTORIES "C:/Compressonator_4.3.206/include"
#  FOLDER Misc
#)

include(compressonator-static)