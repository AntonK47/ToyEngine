set(MODULE_NAME GLSLRuntimeCompiler)
set(MODULE_HEADERS 
	Source/GlslRuntimeCompiler.h

)
set(MODULE_SOURCES
	Source/GlslRuntimeCompiler.cpp
)

add_library(${MODULE_NAME} 
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)

target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
)

find_package(glslang CONFIG REQUIRED)

target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		#GLSL
		SPIRV
		glslang
		
)

if(BUILD_TOY_ENGINE_MODULE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CMakeLists.txt")
    message(STATUS "Build tests for module ${MODULE_NAME}")
    add_subdirectory(Tests)   
endif()
