set(MODULE_NAME Runtime)


set(MODULE_SOURCES 
	Source/Application.cpp
	Source/SceneLoader.cpp
	Source/DDSLoader.cpp
	Source/MapIO.cpp
	Source/TextureManager.cpp
	Source/Material.cpp
	Source/TaskSystem.cpp 
	Source/DynamicFrameAllocator.cpp
	Source/Gui.cpp
)

set(MODULE_HEADERS 
	Source/Application.h
	Source/SceneLoader.h
	Source/DDSLoader.h
	Source/TexturePageExtractor.h 
	Source/DynamicFrameAllocator.h
	Source/IconsFontAwesome6.h
	Source/IconsFontAwesome6Brands.h
	Source/toyimconfig.h
	Source/OutlineFeature.h
	Source/MaterialEditor.h
	Source/Editor.h
	Source/ImageDataUploader.h
	Source/TextureManager.h
	Source/Texture2D.h 
	Source/Material.h 
	Source/BackgroundTasksSystem.h 
	Source/TaskSystem.h
	Source/Gui.h
	Source/Statistics.h
	Source/Utils.h
)

add_library(${MODULE_NAME}
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)



target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		SDLWindow
		RenderInterface
		GLSLRuntimeCompiler
		GraphicsDebugger
		MPMCQueue::MPMCQueue
		Freetype::Freetype
		imgui::imgui
		imgui-node-editor-static
		#Compressator
		#compressonator-static
		#Win32Window
		nlohmann_json::nlohmann_json
)

target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	get_property(RESOURCES GLOBAL "${RESOURCE}" PROPERTY TOY_RESOURCE_DIR)
	if(EXISTS ${RESOURCES})
	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR ${RESOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	else()
	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	endif()
endif()

if(BUILD_TOY_ENGINE_MODULE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CMakeLists.txt")
    message(STATUS "Build tests for module ${MODULE_NAME}")
    add_subdirectory(Tests)   
endif()