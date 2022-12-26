set(MODULE_NAME Engine)
set(MODULE_SOURCES 
	Source/Application.cpp
 "Source/SceneLoader.cpp")
set(MODULE_HEADERS 
	Source/Application.h
 "Source/SceneLoader.h")

add_library(${MODULE_NAME}
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)


target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
)


find_package(glm CONFIG REQUIRED)
find_package(MPMCQueue CONFIG REQUIRED)


target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		SDLWindow
		glm::glm
		VulkanRenderInterface
		GLSLRuntimeCompiler
		GraphicsDebugger
		MPMCQueue::MPMCQueue
		MeshBuilder
		#ozz_animation
		#ozz_animation_fbx
		#ozz_animation_offline
)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
endif()
